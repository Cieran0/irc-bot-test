#include <bot.hpp>
#include <irc.hpp>

int main(int argc, char** argv) {
    std::vector<std::string_view> args;

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args);
}

std::mutex readLock;
std::queue <std::string>readMessages;

std::mutex sendLock;
std::queue <std::string>sendMessages;

std::thread readThread;
std::thread writeThread;

std::unordered_set<std::string> usersInBotChannel;

bool bot::isAlive = false;
/*
    Avoid C style main
*/
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //TODO: HANDLE ERRORS
    bot::details botDetails = bot::getDetailsFromArguments(arguments);
    
    //TODO: HANDLE ERRORS
    bot::clientSocket botSocket = bot::openSocket(botDetails);

    bot::sendInitalMessages(botSocket);

    bot::startThreads(botSocket);

    while (bot::isAlive){
        std::string message = readFromQueue();
        if(message.empty())
            continue; //Should only happen if !bot::isAlive

        irc::command recievedCommand = irc::parseCommand(message);
        
        //parseCommand failed
        if(recievedCommand.name.empty()) {
            std::cerr << "Failed to parse: [" << message << "]" << std::endl;
            return -10;
        }

        //std::cout << "Command: " << recievedCommand.name << std::endl;

        bot::handleCommand(recievedCommand);
    }
    
    if(readThread.joinable())
        readThread.join();
    if(writeThread.joinable())
        writeThread.join();
    return 0;
}


bot::details bot::getDetailsFromArguments(const std::vector<std::string_view>& arguments) {

    //TODO: parse arguments
    bot::details botDetails;
    botDetails.channel = "#";
    botDetails.ip = "::1";
    botDetails.name = "slap_bot";
    botDetails.port = "6667";

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
    struct sockaddr_in6 serverAddress;

    if ((botSocks = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        clean_up();
        return botSocks;
    }

    memset(&serverAddress, 0, sizeof(serverAddress)); // Needed for windows?
    serverAddress.sin6_family = AF_INET6;
    serverAddress.sin6_port = htons(6667);

    if (inet_pton(AF_INET6, "::1", &serverAddress.sin6_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        clean_up();
        return botSocks;
    }

    if ((status = connect(botSocks, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(botSocks);
        clean_up();
        return botSocks;
    }

    return botSocks;
}

void bot::startThreads(bot::clientSocket botSocket) {

    bot::isAlive = true;

    readThread = std::thread(readMessage, botSocket);
    writeThread = std::thread(writeMessage, botSocket);

    readThread.detach();
    writeThread.detach();

}   

void bot::readMessage(bot::clientSocket botSocket) {
    int valread;
    char buffer[1024] = { 0 };

    while (bot::isAlive)
    {
        memset(buffer, 0, sizeof(buffer));
        valread = recv(botSocket, buffer, 1024 -1, 0);
        
           if (valread <= 0 )
            {
                bot::die();
                break;
            }  

        std::vector<std::string> splitByNewline = split_string(std::string(buffer), "\r\n", false);
        readLock.lock();
        for(const std::string& string : splitByNewline) {
            readMessages.push(string);
        }
        readLock.unlock();
    }
    
}  

void bot::writeMessage(bot::clientSocket botSocket) {


    while (bot::isAlive){

    std::string localString;

        sendLock.lock();

            if (!sendMessages.empty())
            {
                localString = sendMessages.front();
                sendMessages.pop();
            }

        sendLock.unlock();

        if (localString.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        send(botSocket, localString.c_str(),localString.length(), 0);
        
    }
    
}  

/*IDK*/
void bot::sendInitalMessages(bot::clientSocket botSocks) {
    std::string initialMessage1 = "NICK stap_bot2\r\n";
    std::string initialMessage2 = "USER slap_bot 0 * :Gamer\r\n";
    std::string initialMessage3 = "JOIN #\r\n";



    bot::addToSendQueue(initialMessage1);
    bot::addToSendQueue(initialMessage2);
    bot::addToSendQueue(initialMessage3);

}

void bot::addToSendQueue(std::string stringToAdd){
       sendLock.lock();
       sendMessages.push(stringToAdd);
       sendLock.unlock();
}

std::string bot::readFromQueue(){

    std::string storedMessage;

    while (storedMessage.empty() && bot::isAlive)
    {
        readLock.lock();
        if(!readMessages.empty()){
            storedMessage = readMessages.front();
            readMessages.pop();
        }
        readLock.unlock();

        if(storedMessage.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    

    return storedMessage;
}

void bot::die() {
    std::cout << "Disconnect from server" << std::endl;
    bot::isAlive = false;
}

std::string concat(std::vector<std::string> vec) {
    if(vec.size() == 0)
        return "";
    std::stringstream ss;
    for (size_t i = 0; i < vec.size()-1; i++)
    {
        ss << vec[i] << " ";
    }
    ss << vec.back();
    return ss.str();
}

void handleServerCommand(std::string hostname, std::vector<std::string> arguments) {

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
                usersInBotChannel.insert(name);
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

void respondToPrivmsg(std::string nickname, std::string channel, std::string text, bool isDm) {
    if(isDm) {
        bot::addToSendQueue("PRIVMSG " + nickname + " :Random sentence goes here\r\n"); //FIXME: make random sentence
        return;
    }

    bot::addToSendQueue("PRIVMSG " + channel + " :"+text+"\r\n");
}

void handleUserCommand(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments) {

    std::string channel = arguments[1];
    std::string text = arguments[2];
    
    if(arguments[0] == "PRIVMSG") 
    {
        std::cout << "Users in channel ";
        for(std::string user : usersInBotChannel) {
            std::cout << user << ", ";
        }
        bool isDm = (channel == "stap_bot2"); //FIXME: dont hard code bot name;
        respondToPrivmsg(nickname, channel, text, isDm);
    }
    else if (arguments[0] == "JOIN") 
    {
        std::cout << nickname << " joined channel: [" << channel << "]" << std::endl;
        usersInBotChannel.insert(nickname);
    } 
    else if (arguments[0] == "PART") 
    {
        std::cout << nickname << " left channel: [" << channel << "]" << std::endl;
        if(nickname != "stap_bot2") {//FIXME: dont hard code bot name
            usersInBotChannel.erase(nickname);
        }
    } 
    else if (arguments[0] == "QUIT") 
    {
        std::cout << nickname << " quit with message : [" << arguments[1] << "]" << std::endl;
        if(nickname != "stap_bot2") {//FIXME: dont hard code bot name
            usersInBotChannel.erase(nickname);
        }
    }
    else 
    {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
        //FIXME: crash on unknown commands;
        exit(-1);
    }
}

void bot::handleCommand(irc::command commandToHandle) {
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
            handleUserCommand(nickname,username,ip, commandToHandle.arguments);
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
        std::cout << "PING" << std::endl;
        std::string raw = commandToHandle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::addToSendQueue(raw);
    } 
    else 
    {
        std::cout << "Uknown command: "<< commandToHandle.name << std::endl;
    }
}