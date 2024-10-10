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
#include <regex>
#include <unordered_set>
#include <irc.hpp>
#include <irc_numberic_replies.hpp>
#include <util.hpp>
#include <fstream>

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

    //bot.cpp
    
    int main(const std::string_view& program, const std::vector<std::string_view>& arguments);
    void die();

    //bot_details.cpp

    bot::details get_details_from_arguments(const std::vector<std::string_view> &arguments);
    void get_channel(bot::details &bot_details, const std::vector<std::string_view> &arguments, size_t i);
    void get_name(const std::vector<std::string_view> &arguments, size_t i, bot::details &bot_details);
    void get_port(const std::vector<std::string_view> &arguments, size_t i, bot::details &bot_details);
    
    //bot_io.cpp

    bot::clientSocket open_socket(const bot::details &botDetails);
    std::string read_message(bot::clientSocket bot_socket);
    void send_message(std::string message, bot::clientSocket bot_socket);
    void send_inital_message(bot::clientSocket bot_socket, bot::details botDetails);
    void respond_to_private_message(std::string nickname, std::string channel, std::string text, bool isDm, bot::details botDetails, bot::clientSocket bot_socket);

    //random.cpp

    std::string get_random_user(const std::string& sender, bot::details botInfo);
    std::string get_random_fact();

    //user_and_server_commands.cpp

    void handle_command(irc::command command_to_handle, bot::details botDetails, bot::clientSocket bot_socket);
    void handle_server_command(std::string hostname, std::vector<std::string> arguments);
    void handle_user_command(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments, bot::clientSocket bot_socket,  bot::details botDetails);
    
    //bot_commands.cpp

    void slap_command(std::string &text, std::string &nickname, const bot::details &bot_details, std::string &response);
    void topic_command(std::string &text, std::string &channel, bot::clientSocket bot_socket, const bot::details &bot_details, std::string &response);

    extern bool is_alive;
    extern std::unordered_set<std::string> users_in_bot_channel;
}