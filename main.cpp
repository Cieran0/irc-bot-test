#include <bot_new.hpp>


int main(int argc, const char** argv) {
    std::vector<std::string_view> args;

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args);
}

bool bot::isAlive = false;
/*
    Avoid C style main
*/
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //TODO: HANDLE ERRORS
    bot::details botDetails = bot::getDetailsFromArguments(arguments);
    
    //TODO: HANDLE ERRORS
    bot::clientSocket botSocket = bot::openSocket(botDetails);

    bot::isAlive = true;
    bot::sendInitalMessages(botSocket, botDetails);

    while (bot::isAlive){
        std::string message = bot::readMessage(botSocket);
        std::string isPing = "PING";
        std::string isMessage = "PRIVMSG";

        if (message.find(isPing) != std::string::npos)
        {
            std::cout << "PONG" << std::endl;
            bot::sendMessage("PONG :DESKTOP-JPO54SA.wireless.dundee.ac.uk\r\n", botSocket); 
        }

        else if (message.find(isMessage) != std::string::npos)
        {
            bot::respondToMessages(message, botSocket);
        }

        else if (message.empty()){
            continue; //Should only happen if !bot::isAlive
        }
    }
    
    return 0;
}


bot::details bot::getDetailsFromArguments(const std::vector<std::string_view>& arguments) {

    //TODO: parse arguments
    bot::details botDetails;
    botDetails.channel = "#";
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
    std::cout << "CONNECTING..." << std::endl;
    if ((status = connect(botSocks, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(botSocks);
        clean_up();
        return botSocks;
    }
    std::cout << "CONNECTed!" << std::endl;


    return botSocks;
}


void bot::pong(std::string messageRecieved){



}

std::string bot::readMessage(bot::clientSocket botSocket) {
    char buffer[1024] = { 0 };
    int valread = recv(botSocket, buffer, 1024 -1, 0);
        
    if (valread <= 0 )
    {
        bot::die();
        return std::string();
    }  
    
    return std::string(buffer);
}  

std::vector<std::string> bot::getUsersInChannel(const std::string& sender, const std::string& botName, const std::string& serverResponse) {
    std::vector<std::string> users;
    std::istringstream responseStream(serverResponse);
    std::string line;

    while (std::getline(responseStream, line)) {
        if (line.find(" 352 ") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string token;
            std::vector<std::string> tokens;
            while (lineStream >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 4) {
                std::string username = tokens[4];
               
                if (username != sender && username != botName) {
                    users.push_back(username);
                }
            }
        } else if (line.find(" 315 ") != std::string::npos) {
            // No more users to parse in channel
            break;
        }
    }

    return users;
}

std::string bot::getRandomUser(const std::string& sender, const std::vector<std::string>& users) {
    std::vector<std::string> eligibleUsers;

    std::vector<std::string_view> arguments = {};
    bot::details botInfo = bot::getDetailsFromArguments(arguments);

    for (const std::string& user : users) {
        if (user != sender && user != botInfo.name) {
            eligibleUsers.push_back(user);
        }
    }

    if (eligibleUsers.empty()) {
        return "";
    }

    srand(static_cast<unsigned>(time(0)));
    int randomIndex = rand() % eligibleUsers.size();

    return eligibleUsers[randomIndex];
}

void bot::respondToMessages(std::string messageRecieved, bot::clientSocket botSocket){
    std::vector<std::string_view> arguments = {};
    bot::details botInfo = bot::getDetailsFromArguments(arguments);
    std::vector<std::string> random_facts = {
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

    std::string name, message; 

    std::cout << messageRecieved << std::endl;

    message = bot::parseMessage(messageRecieved);
    name = bot::readName(messageRecieved);

    std::cout << name << message << std::endl;

    bot::sendMessage("WHO #\r\n",botSocket);

    std::string serverResponse = bot::readMessage(botSocket);

    if (!message.empty() && message[0] == '!') {
        std::istringstream iss(message);
        std::random_device rand;
        std::string command;
        iss >> command;

        if (command == "!hello") {
            std::uniform_int_distribution<> dis(0, random_facts.size() - 1);
            std::string message = "PRIVMSG # :Hello, " + name + " :" + random_facts[dis(rand)] + "\r\n";
            bot::sendMessage(message,botSocket);
        }
        else if (command == "!slap") {
            std::vector<std::string> recipients;
            std::string recipient;

            iss >> recipient;

            std::vector<std::string> usersInChannel = getUsersInChannel(name, std::string{botInfo.name}, serverResponse);

            // if (usersInChannel.empty()) {
            //     std::cout << "No users in the channel." << std::endl;
            // } else {
            //     std::cout << "Users in the channel:" << std::endl;

            //     for (const std::string& user : usersInChannel) {
            //         std::cout << user << std::endl;
            //     }
            // }

            if (!recipient.empty()) {
                if (std::find(usersInChannel.begin(), usersInChannel.end(), recipient) != usersInChannel.end()) {
                    bot::sendMessage("PRIVMSG # :" + name + " slaps " + recipient + " with a large trout!\r\n", botSocket);
                } else {
                    bot::sendMessage("PRIVMSG # :" + name + " slaps themselves with a large trout for trying to slap someone not in the channel!\r\n", botSocket);
                }
            } else {
                std::string randomUser = getRandomUser(name, usersInChannel);
                if (!randomUser.empty()) {
                    bot::sendMessage("PRIVMSG # :" + name + " slaps " + randomUser + " with a large trout!\r\n", botSocket);
                } else {
                    bot::sendMessage("PRIVMSG # :There's no one to slap in the channel!\r\n", botSocket);
                }
            }
        }
    }
        
}

std::string bot::parseMessage(std::string messageRecieved){

   std::string delimiter = "# :";

        size_t pos = messageRecieved.find(delimiter);

    if (pos != std::string::npos)
    {
        pos += delimiter.length();

        std::string message = messageRecieved.substr(pos);

        size_t first = message.find_first_not_of(' ');

            if (first != std::string::npos)
            {
                message = message.substr(first);
                return message;
            }
    }

    return "error";
}

std::string bot::readName(std::string messageRecieved){

    std::regex pattern(":([^!]+)!");
    std::smatch match;

    if(std::regex_search(messageRecieved, match, pattern)){
        std::string name = match[1];
        return name;
    }else{
        std::cout << "couldnt find name " << std::endl;
    }

    return "error";

}

void bot::sendMessage(std::string message, bot::clientSocket botSocket) {
    if (bot::isAlive){
        send(botSocket, message.c_str(),message.length(), 0);
    }
}  

/*IDK*/
void bot::sendInitalMessages(bot::clientSocket botSocks, bot::details botDetails) {
    std::string initialMessage1 = "NICK " + std::string(botDetails.name) + "\r\n";
    std::string initialMessage2 = "USER " + std::string(botDetails.name) + " 0 * :Gamer\r\n";
    std::string initialMessage3 = "JOIN #\r\n";



    bot::sendMessage(initialMessage1,botSocks);
    bot::sendMessage(initialMessage2,botSocks);
    bot::sendMessage(initialMessage3,botSocks);

}

void bot::die() {
    std::cout << "Disconnect from server" << std::endl;
    bot::isAlive = false;
}