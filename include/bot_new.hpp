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
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
   
#endif


int main(int argc, char** argv);

namespace bot {

    #ifdef _WIN32
        typedef SOCKET socket;  // Use SOCKET on Windows
        #define close_socket(s) closesocket(s)
    #else
        typedef int socket;  // Use int on Linux
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
    bot::socket openSocket(const bot::details& botDetails);
    void handleSocket(bot::socket botSocket);

    /*IDK?*/

    void sendInitalMessages(/*args*/);
    std::string getNextMessage(/*args*/);
    void handleMessage(const std::string& message/*args*/);
    void sendMessage(/*args*/);

    extern bool isAlive;
}