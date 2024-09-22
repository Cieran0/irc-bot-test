#include <bot.hpp>
#include <thread>
#include <unistd.h>
#include <cstring> // For strerror()
#include <cerrno>  // For errno
#include <util.hpp>

#ifdef _WIN32
    #define clean_up() WSACleanup()
#else
    #define clean_up() 
#endif

#define wait_in_millisecond(x) usleep(x*1000)

bot::bot(std::string serverIp, int port, std::string username, std::string channel) 
: serverIp(serverIp), port(port), username(username), channel(channel)
{
    initializeConnection();
}

bot::~bot()
{
    close_socket(m_bot_socket);
    clean_up();

    if (m_input_thread.joinable()) {
        m_input_thread.join();
    }
    if (m_output_thread.joinable()) {
        m_output_thread.join();
    }
}

std::string bot::get_next_message() {
    std::string message;
    while (message.empty() && m_alive) {
        {
            const std::lock_guard<std::mutex> lock(m_input_lock);
            if (!m_input_buffer.empty()) {
                message = m_input_buffer.front();
                m_input_buffer.pop();
            }
        }
        wait_in_millisecond(5); 
    }
    return message;
}

void bot::send_message(const std::string message) {
    if (message.empty())
        return;

    const std::lock_guard<std::mutex> lock(m_output_lock);
    m_output_buffer.push(message);
}

void bot::handle_input(bot* bot_to_handle) {
    char buffer[1024];
    int bytes_received = -1;

    while (bot_to_handle->m_alive) {
        bytes_received = recv(bot_to_handle->m_bot_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Received: {" << std::endl << buffer << "}" << std::endl;
            {
                std::lock_guard<std::mutex> lock(bot_to_handle->m_input_lock);
                std::string raw_string = std::string(buffer);

                std::vector<std::string> split_by_newline = split_string(raw_string, "\r\n", false);
                for(const std::string& string : split_by_newline) {
                    bot_to_handle->m_input_buffer.push(string + "\r\n");
                }
            }
        } else {
            if (bytes_received == 0) {
                std::cout << "Connection closed by server." << std::endl;
                bot_to_handle->die();
            } else {
                std::cerr << "recv failed with error: " << strerror(errno) << std::endl;
                bot_to_handle->die();
            }
            break;
        }
    }
}

void bot::handle_output(bot* bot_to_handle) {
    while (bot_to_handle->m_alive) {
        std::string message;
        {
            std::lock_guard<std::mutex> lock(bot_to_handle->m_output_lock);
            if (!bot_to_handle->m_output_buffer.empty()) {
                message = bot_to_handle->m_output_buffer.front();
                bot_to_handle->m_output_buffer.pop();
            }
        }

        if (message.empty()) {
            wait_in_millisecond(5);
            continue;
        }

        int sendResult = send(bot_to_handle->m_bot_socket, message.c_str(), message.length(), 0);
        if (sendResult == -1) {
            std::cerr << "Failed to send message." << std::endl;
            close_socket(bot_to_handle->m_bot_socket);
            clean_up();
            return;
        }
        std::cout << "Message sent to server: " << message << std::endl;
    }
}

int bot::initializeConnection() {
    #ifdef _WIN32
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return 1;
        }
    #endif

    // Create the socket (IPv6)
    m_bot_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (m_bot_socket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        clean_up();
        return 1;
    }

    // Specify server address (IPv6 localhost)
    sockaddr_in6 serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(port);
    inet_pton(AF_INET6, serverIp.c_str(), &serverAddr.sin6_addr);

    // Connect to the server
    if (connect(m_bot_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to connect: " << strerror(errno) << std::endl;
        close_socket(m_bot_socket);
        clean_up();
        return 1;
    }

    m_alive = true;

    m_input_thread = std::thread(handle_input, this);
    m_output_thread = std::thread(handle_output, this);

    m_input_thread.detach();
    m_output_thread.detach();


    return 0;
}

void bot::die() {
    m_alive = false;
}

bool bot::alive() {
    return m_alive;
}