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