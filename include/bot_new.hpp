#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <cstring>
#include <string_view>
#include <algorithm>
#include <sstream>
#include <vector>
#include <random>
#include <cstdlib>
#include <ctime>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define clean_up() WSACleanup()
    #define close_socket(s) closesocket(s)  // Windows uses closesocket()
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define clean_up() 
    #define close_socket(s) close(s)  // Linux uses close()
#endif


int main(int argc, const char** argv);

namespace bot {

    #ifdef _WIN32
        typedef SOCKET clientSocket;  // Use SOCKET on Windows
        #define close_socket(s) closesocket(s)
    #else
        typedef int clientSocket;  // Use int on Linux
        #define close_socket(s) close(s)
    #endif

    struct details {
        std::string_view name;
        std::string_view ip;
        std::string_view port;
        std::string_view channel;
    };

    int main(const std::string_view& program, const std::vector<std::string_view>& arguments);

    bot::details getDetailsFromArguments(const std::vector<std::string_view>& arguments);
    bot::clientSocket openSocket(const bot::details& botDetails);

    std::string readMessage(bot::clientSocket botSocket);
    void sendMessage(std::string message, bot::clientSocket botSocket); 

    void pong(std::string messageRecieved);
    void respondToMessages(std::string messageRecieved, bot::clientSocket botSocket);
    std::vector<std::string> getUsersInChannel(const std::string& sender, const std::string& botName, const std::string& serverResponse);
    std::string getRandomUser(const std::string& sender, const std::vector<std::string>& users);
    std::string parseMessage(std::string messageRecieved);
    std::string readName(std::string messageRecieved);

    void die();

    void sendInitalMessages(bot::clientSocket botSocket, bot::details botDetails);

    extern bool isAlive;
}