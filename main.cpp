#include <bot_new.hpp>


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
    
    bot::details botDetails = bot::getDetailsFromArguments(arguments);
    
    bot::clientSocket botSocket = bot::openSocket(botDetails);

    bot::sendInitalMessages(botSocket);

    bot::startThreads(botSocket);

    while (true){
        std::cout << readFromQueue() << "\n";
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

    return botDetails;
}

bot::clientSocket bot::openSocket(const bot::details& botDetails) {

    bot::clientSocket botSocks = 0;

    //TODO: connect to server

    int status, valread;
    struct sockaddr_in6 serv_addr;
    std::string hello = "Ive connected!\n";
    char buffer[1024] = { 0 };

    if ((botSocks = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return -1;
    }

    std::string port = std::string(botDetails.port);

    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(stoi(port, 0, 10));

    if (inet_pton(AF_INET6, "::1", &serv_addr.sin6_addr) <= 0) 
    {
       std::cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if ((status = connect(botSocks, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) 
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    return botSocks;
}

void bot::startThreads(bot::clientSocket botSocket) {

    readThread = std::thread(readMessage, botSocket);
    writeThread = std::thread(writeMessage, botSocket);

    readThread.detach();
    writeThread.detach();

}   

void bot::readMessage(bot::clientSocket botSocket) {
    int valread;
    char buffer[1024] = { 0 };

    while (true)
    {
        valread = read(botSocket, buffer, 1024 -1);
        
           if (valread <= 0 )
            {
                    std::cout << "NOOOOOO!!!\n";  
                    break;
            }  

        readLock.lock();
        readMessages.push(std::string(buffer));
        readLock.unlock();
    }
    
}  

void bot::writeMessage(bot::clientSocket botSocket) {


    while (true){

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
            usleep(5000);
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

    while (storedMessage.empty())
    {
        readLock.lock();
        if(!readMessages.empty()){
            storedMessage = readMessages.front();
            readMessages.pop();
        }
        readLock.unlock();

        if(storedMessage.empty()) {
            usleep(5000);
        }
    }
    

    return storedMessage;
}