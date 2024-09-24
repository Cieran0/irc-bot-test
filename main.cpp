#include <bot_new.hpp>
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

        std::cout << "Command: " << recievedCommand.name << std::endl;

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
    struct sockaddr_in6 serv_addr;

    if ((botSocks = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        clean_up();
        return botSocks;
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); // Needed for windows?
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(6667);

    if (inet_pton(AF_INET6, "::1", &serv_addr.sin6_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        clean_up();
        return botSocks;
    }

    if ((status = connect(botSocks, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
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

        std::vector<std::string> split_by_newline = split_string(std::string(buffer), "\r\n", false);
        readLock.lock();
        for(const std::string& string : split_by_newline) {
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

void handleServerCommand(std::string hostname, std::vector<std::string> arguments) {
    //std::cout << "HOSTNAME [" << hostname << "]" << std::endl;
    //for (size_t i = 0; i < arguments.size(); i++)
    //{
    //    std::cout << "ARG"<<i<<"["<<arguments[i]<<"]" << std::endl;
    //}
    irc::numeric_reply num;
    if(!irc::isKnownNumericReply(arguments[0])) {
        std::cout << "UKNKOWN" << arguments[0] << std::endl;
        //FIXME: crash on uknown thing
        exit(-1);
    }

    num = (irc::numeric_reply)std::stoi(arguments[0]);

    if(num == irc::numeric_reply::RPL_WELCOME) {
        //TODO: do stuff here maybe
    }
    else if (num == irc::numeric_reply::RPL_YOURHOST) {
        //TODO: do stuff here maybe
    }
    else if (num == irc::numeric_reply::RPL_ENDOFNAMES) {
        //TODO: do stuff here maybe
    } else if (num == irc::numeric_reply::RPL_NOTOPIC) {    
        //TODO: do stuff here maybe
    } else if (num == irc::RPL_NAMREPLY) {
        //TODO: do stuff here maybe
    }
    else {
        std::cout << "Unhandled: " << irc::validNumericReplies.find(num)->second << std::endl;
    }
}

void handleUserCommand(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments) {
    //std::cout << "NICK [" << nickname << "]" << std::endl;
    //std::cout << "USERNAME [" << username << "]" << std::endl;
    //std::cout << "IP [" << ip << "]" << std::endl;
    //std::cout << "COMMAND [" << arguments[0] << "]" << std::endl;
    //for (size_t i = 1; i < arguments.size(); i++)
    //{
    //    std::cout << "ARG"<<i<<"["<<arguments[i]<<"]" << std::endl;
    //}

    if(arguments[0] == "PRIVMSG") {
        std::string channel = arguments[1];
        std::string text = arguments[2];
        std::string response;
        //FIXME: dont hard code nickname
        if(channel == "stap_bot2") {
            //privmsg to bot
            //std::cout << nickname << " said " << text << " in dms" << std::endl;
            response = "Random sentence";
            channel = nickname;
        } else {
            //in a channel
            //std::cout << nickname << " said " << text << " in " <<channel << std::endl;
            response = text;
        }

        std::string out = "PRIVMSG "+channel+" :" + response+"\r\n";
        bot::addToSendQueue(out);
    }
    else if (arguments[0] == "JOIN") {
        std::string channel = arguments[1];
        std::cout << "joined channel: [" << channel << "]" << std::endl;
    } 
    else {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
        //FIXME: crash on unknown commands;
        exit(-1);
    }
}

void bot::handleCommand(irc::command commandToHandle) {
    if(commandToHandle.name.starts_with(":")) {
        std::string server_or_user_nickname = commandToHandle.name.substr(1,commandToHandle.name.length()-1);
        std::cout << server_or_user_nickname << std::endl;
        size_t end_of_nickname = server_or_user_nickname.find('!');
        if(end_of_nickname != std::string::npos){
            //USER
            size_t end_of_user_name = server_or_user_nickname.find('@');
            std::string nickname = server_or_user_nickname.substr(0,end_of_nickname);
            std::string username = server_or_user_nickname.substr(end_of_nickname+1, end_of_user_name-end_of_nickname-1);
            std::string ip = server_or_user_nickname.substr(end_of_user_name+1);
            handleUserCommand(nickname,username,ip, commandToHandle.arguments);
        } else {
            //Server
            std::string hostname = server_or_user_nickname;
            handleServerCommand(hostname, commandToHandle.arguments);
        }
    } else if (commandToHandle.name == "PING") {
        std::cout << "PING" << std::endl;
        std::string raw = commandToHandle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::addToSendQueue(raw);
    } else {
        std::cout << "Uknown command: "<< commandToHandle.name << std::endl;
    }
}