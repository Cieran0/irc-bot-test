#include <bot_new.hpp>

int main(int argc, const char** argv) {
    std::vector<std::string_view> args;

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args);
}

bool bot::isAlive = false;
std::unordered_set<std::string> bot::usersInBotChannel;


int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //TODO: HANDLE ERRORS
    bot::details botDetails = bot::getDetailsFromArguments(arguments);
    
    //TODO: HANDLE ERRORS
    bot::clientSocket botSocket = bot::openSocket(botDetails);

    bot::isAlive = true;
    bot::sendInitalMessages(botSocket, botDetails);

    while (bot::isAlive){
        std::string message = bot::readMessage(botSocket);

        if (message.empty()){
            continue; //Should only happen if !bot::isAlive
        }

        irc::command recievedCommand = irc::parseCommand(message);
        
        //parseCommand failed
        if(recievedCommand.name.empty()) {
            std::cerr << "Failed to parse: [" << message << "]" << std::endl;
            return -10;
        }

        //std::cout << "Command: " << recievedCommand.name << std::endl;

        bot::handleCommand(recievedCommand, botDetails, botSocket);

    }
    
    return 0;
}


bot::details bot::getDetailsFromArguments(const std::vector<std::string_view>& arguments) {

    bot::details botDetails;
    botDetails.channel = "";
    botDetails.ip = "::1";
    botDetails.name = "slap_bot";
    botDetails.port = "6667";

    for (size_t i = 0; i < arguments.size(); i+=2)
    {
        if(arguments.size() < i+2){
            std::cerr << "Invalid amount of args" << std::endl;
            exit(-1);
        }
        else if(arguments[i] == "--host") {
            botDetails.ip = arguments[i+1];
        } 
        else if (arguments[i] == "--port")
        {
            botDetails.port = arguments[i+1];   
        }
        else if (arguments[i] == "--name")
        {
            botDetails.name = arguments[i+1];
        }
        else if (arguments[i] == "--channel")
        {
            botDetails.channel = arguments[i+1];
        }
        
        else {
            std::cerr << "Invalid arg " << arguments[i];
            exit(-1);
        }
    }
    

 

    return botDetails;
}

bot::clientSocket bot::openSocket(const bot::details& botDetails) {
    bot::clientSocket botSocks = -1;

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return botSocks;
    }
#endif

    int status;
    struct sockaddr_in6 serv_addr;

    if ((botSocks = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        clean_up();
        return botSocks;
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(6667);

    if (inet_pton(AF_INET6, "::1", &serv_addr.sin6_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        clean_up();
        return botSocks;
    }
    std::cout << "Connecting..." << std::endl;
    if ((status = connect(botSocks, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(botSocks);
        clean_up();
        return botSocks;
    }
    std::cout << "Connected!" << std::endl;


    return botSocks;
}

std::queue<std::string> readQueue;
 
std::string bot::readMessage(bot::clientSocket botSocket) {
    if(!readQueue.empty()) {
        std::string out = readQueue.front();
        readQueue.pop();
        return out;
    }
    
    char buffer[1024] = { 0 };
    int valread = recv(botSocket, buffer, 1024 -1, 0);
        
    if (valread <= 0 )
    {
        bot::die();
        return std::string();
    }  
    
    std::string messages = std::string(buffer);

    for(const std::string& s: split_string(messages,"\r\n",false)) {
        readQueue.push(s);
    }
    if(!readQueue.empty()) {
        std::string out = readQueue.front();
        readQueue.pop();
        return out;
    }
    return std::string();
}  

std::string bot::getRandomUser(const std::string& sender, bot::details botInfo) {
    std::vector<std::string> eligibleUsers;

    for (const std::string& user : bot::usersInBotChannel) {
        if (user != sender && user != botInfo.name) {
            eligibleUsers.push_back(user);
        }
    }

    if (eligibleUsers.empty()) {
        return std::string();
    }

    return eligibleUsers[0];
}

void bot::sendMessage(std::string message, bot::clientSocket botSocket) {
    if (bot::isAlive){
        send(botSocket, message.c_str(),message.length(), 0);
    }
}  

/*IDK*/
void bot::sendInitalMessages(bot::clientSocket botSocks, bot::details botDetails) {
    std::string initialMessage1 = "NICK " + std::string(botDetails.name) + "\r\n";
    std::string initialMessage2 = "USER " + std::string(botDetails.name) + " 0 * :"+std::string(botDetails.name)+"\r\n";
    std::string initialMessage3 = "JOIN #" + std::string(botDetails.channel) + "\r\n";

    bot::sendMessage(initialMessage1,botSocks);
    bot::sendMessage(initialMessage2,botSocks);
    bot::sendMessage(initialMessage3,botSocks);

}

void bot::die() {
    std::cout << "Disconnect from server" << std::endl;
    bot::isAlive = false;
}

void bot::handleServerCommand(std::string hostname, std::vector<std::string> arguments) {

    irc::numeric_reply numericReply;
    if(!irc::isKnownNumericReply(arguments[0])) {
        std::cout << "UKNKOWN" << arguments[0] << std::endl;
        //FIXME: crash on uknown thing
        exit(-1);
    }

    numericReply = (irc::numeric_reply)std::stoi(arguments[0]);

    switch (numericReply) {
        case irc::RPL_WELCOME:
        case irc::RPL_YOURHOST:
        case irc::RPL_NOTOPIC:
        case irc::RPL_CREATED:
        case irc::RPL_MYINFO:
        case irc::ERR_NOMOTD:
        case irc::RPL_LUSERCLIENT:
            //print this
            break;
        case irc::RPL_NAMREPLY: {
            std::string namesRaw = arguments.back();
            std::vector<std::string> names = split_string(namesRaw, " ", true);
            for(const std::string& name : names) {
                bot::usersInBotChannel.insert(name);
            }
            break;
        }
        case irc::RPL_ENDOFNAMES:
            //ignore this, we know when names are over i think?
            break;
        default:
            std::cout << "Unhandled: " << irc::validNumericReplies.find(numericReply)->second << std::endl;
            break;
    }
}



std::string getRandomSentence() {
    const std::string random_facts[] = {
    "The Planck length, the smallest measurable length in physics, is about 1.6 x 10^-35 meters.",
    "Mozart composed a piece called 'A Musical Joke' (K. 522), which intentionally uses awkward harmonies and rhythms to mock incompetent musicians.",
    "Bananas are technically berries, while strawberries are not.",
    "In 1977, a signal from space known as the 'Wow! signal' was detected by a radio telescope. Its origins remain unexplained.",
    "Wombat poop is cube-shaped to help it stack neatly and mark territory.",
    "Sharks existed before trees. Sharks have been around for more than 400 million years, while trees appeared around 350 million years ago.",
    "Octopuses have three hearts. Two pump blood to the gills, and one pumps it to the rest of the body.",
    "The first oranges weren't orange. They were green and came from Southeast Asia, where they were a cross between a pomelo and a mandarin.",
    "There's a species of jellyfish, Turritopsis dohrnii, that is effectively immortal because it can revert back to its juvenile form after reaching adulthood.",
    "The Eiffel Tower can grow up to 6 inches taller in the summer due to the expansion of the metal when heated."
    };

    int num_facts = sizeof(random_facts) / sizeof(random_facts[0]);

    srand((time(NULL)));
    return random_facts[rand() % num_facts];
}

void bot::respondToPrivmsg(std::string nickname, std::string channel, std::string text, bool isDm, bot::details botDetails, bot::clientSocket botSocket) {
    
    std::cout << "text is: " << text << std::endl;
    std::cout << "nickname is: " << nickname << std::endl;

     if (text.find("PRIVMSG") != std::string::npos) {
       
        std::cout << "Received a PRIVMSG command." << std::endl;
        bot::sendMessage("PRIVMSG " + nickname + " :"+getRandomSentence()+"\r\n", botSocket); 

    }

    /*if(isDm) {
        bot::sendMessage("PRIVMSG " + nickname + " :"+getRandomSentence()+"\r\n", botSocket); //FIXME: make random sentence
        return;
    } */

    
    if(text[0] == '!') {
        std::string responce;

        if(text == "!slap") {
            std::string user = getRandomUser(nickname, botDetails);
            if(user.empty()) {
                responce = "There's no one to slap!";
            } else {
                responce = "*Slapped " + user + " with a trout*";
            }
        } else if (text.starts_with("!slap ")) {
            std::string user = split_string(text, " ", true)[1];
            if(bot::usersInBotChannel.find(user) != bot::usersInBotChannel.end()) {
                responce = "*Slapped " + user + " with a trout*";
            } else {
                responce = "*Slapped " + nickname + " with a trout* since " + user + " couldn't be found!";
            }
        } else if (text == "!hello") {
            responce = "Hey, " + nickname;
        }
 
        
        if(responce.empty())
            return;
        bot::sendMessage("PRIVMSG " + channel + " :"+responce+"\r\n", botSocket);
    }

}

void bot::handleUserCommand(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments, bot::clientSocket botSocket, bot::details botDetails) {

    std::string channel = arguments[1];
    
    if(arguments[0] == "PRIVMSG") 
    {
        std::string text = arguments[2];
        bool isDm = (channel == botDetails.name);
        respondToPrivmsg(nickname, channel, text, isDm, botDetails, botSocket);
    }
    else if (arguments[0] == "JOIN") 
    {
        std::cout << nickname << " joined channel: [" << channel << "]" << std::endl;
        bot::usersInBotChannel.insert(nickname);
    } 
    else if (arguments[0] == "PART") 
    {

        std::cout << nickname << " left channel: [" << channel << "]" << std::endl;
        if(nickname != botDetails.name) {
            bot::usersInBotChannel.erase(nickname);
        }
    } 
    else if (arguments[0] == "QUIT") 
    {
        std::cout << nickname << " quit with message : [" << arguments[1] << "]" << std::endl;
        if(nickname != "stap_bot2") {//FIXME: dont hard code bot name
            bot::usersInBotChannel.erase(nickname);
        }
    }
    else 
    {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
        //FIXME: crash on unknown commands;
        exit(-1);
    }
}

void bot::handleCommand(irc::command commandToHandle, bot::details botDetails, bot::clientSocket botSocket ) {
    if(commandToHandle.name.starts_with(":")) 
    {
        std::string hostnameOrUserNickname = commandToHandle.name.substr(1,commandToHandle.name.length()-1);
        //std::cout << hostnameOrUserNickname << std::endl;
        size_t endOfNickname = hostnameOrUserNickname.find('!');
        if(endOfNickname != std::string::npos)
        {
            //USER
            std::string userRaw = hostnameOrUserNickname;
            size_t endOfUserName = userRaw.find('@');
            std::string nickname = userRaw.substr(0,endOfNickname);
            std::string username = userRaw.substr(endOfNickname+1, endOfUserName-endOfNickname-1);
            std::string ip = userRaw.substr(endOfUserName+1);
            handleUserCommand(nickname,username,ip, commandToHandle.arguments, botSocket, botDetails);
        } 
        else 
        {
            //Server
            std::string hostname = hostnameOrUserNickname;
            handleServerCommand(hostname, commandToHandle.arguments);
        }
    } 
    else if (commandToHandle.name == "PING") 
    {
        //std::cout << "PING" << std::endl;
        std::string raw = commandToHandle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::sendMessage(raw, botSocket);
    } 
    else 
    {
        std::cout << "Uknown command: "<< commandToHandle.name << std::endl;
    }
}