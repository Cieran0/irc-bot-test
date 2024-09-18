#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET bot_socket;  // Use SOCKET on Windows
    #define close_socket(s) closesocket(s)  // Windows uses closesocket()
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int bot_socket;  // Use int on Linux
    #define close_socket(s) close(s)  // Linux uses close()
#endif

class bot
{
private:
    std::string serverIp;
    int port;
    std::string username;
    std::string channel;

    std::queue<std::string> m_input_buffer;
    std::queue<std::string> m_output_buffer;
    std::mutex m_input_lock;
    std::mutex m_output_lock;


    std::thread m_input_thread;
    std::thread m_output_thread;

    static void handle_input(bot* bot_to_handle);
    static void handle_output(bot* bot_to_handle);

    bot_socket m_bot_socket;
    

public:
    bot(std::string serverIp, int port, std::string username, std::string channel);
    ~bot();
    int initializeConnection();

    std::string get_next_message();
    void send_message(const std::string message);

};
