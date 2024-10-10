#include <bot.hpp>

/*
    Opens a socket connection to the IRC server
*/
bot::clientSocket bot::open_socket(const bot::details& bot_details) {
    bot::clientSocket bot_socket = -1; // Initialize the socket

    #ifdef _WIN32
        // Windows specific socket initialization
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return bot_socket; // Return failure
        }
    #endif

    int status;
    struct sockaddr_in6 serv_addr; // Structure for IPv6 addresses

    // Create the socket
    if ((bot_socket = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        clean_up(); 
        return bot_socket; // Returns a failure 
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); // Clears the previously created struct
    serv_addr.sin6_family = AF_INET6; // Sets to IPv6 to avoid IPv4 default
    serv_addr.sin6_port = htons(std::stoi(std::string(bot_details.port))); // Sets the port

    // Converts IP address from text to binary 
    if (inet_pton(AF_INET6, std::string(bot_details.ip).c_str(), &serv_addr.sin6_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl; 
        clean_up(); 
        return bot_socket; 
    }
    std::cout << "Connecting..." << std::endl; 

    if ((status = connect(bot_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(bot_socket); 
        clean_up(); 
        return bot_socket;
    }
    std::cout << "Connected!" << std::endl; // Print statement for a successful connection


    return bot_socket;// Return the valid socket
}

// Queue to store recieved messages from the server 
std::queue<std::string> read_queue;
 
/*
    Reads a message from the read queue.
    If the read queue is empty reads a message from server.
*/
std::string bot::read_message(bot::clientSocket bot_socket) {

    // Check to see if there are messages currently stored in the queue
    if(!read_queue.empty()) {
        std::string out = read_queue.front(); // Get the top message of the queue
        read_queue.pop(); // Remove it from the queue 
        return out; // Return the message 
    }
    
    char buffer[1024] = { 0 }; // Buffer to store the message 
    int number_of_bytes_read = recv(bot_socket, buffer, 1024 -1, 0); // Read from the socket 
        
    // Check for errors of disconnection from the server    
    if (number_of_bytes_read <= 0 )
    {
        bot::die(); // Terminate the bot
        return std::string(); // Return an empty string 
    }  
    
    std::string messages = std::string(buffer); // Convert the buffer to a string 

    // Split the messages and pass them to the read_queue
    for(const std::string& s: split_string(messages,"\r\n",false)) {
        read_queue.push(s);
    }

    return bot::read_message(bot_socket);
}  

/*
    Sends message to IRC server on bot_socket
*/
void bot::send_message(std::string message, bot::clientSocket bot_socket) {
    // Checks to see if bot is alive first 
    if (bot::is_alive){
        // Uses the socket to send a message 
        send(bot_socket, message.c_str(),message.length(), 0);
    }
}  

/*
    Sends the intial messages to the IRC server.
*/
void bot::send_inital_message(bot::clientSocket bot_socket, bot::details bot_details) {
    std::string nick_message = "NICK " + std::string(bot_details.name) + "\r\n";
    std::string user_message = "USER " + std::string(bot_details.name) + " 0 * :"+std::string(bot_details.name)+"\r\n";
    std::string join_message = "JOIN " + std::string(bot_details.channel) + "\r\n";

    bot::send_message(nick_message,bot_socket);
    bot::send_message(user_message,bot_socket);
    bot::send_message(join_message,bot_socket);

}

/*
    Handle responses to PRIVMSG from users. 
*/ 
void bot::respond_to_private_message(std::string nickname, std::string channel, std::string text, bool isDm, bot::details bot_details, bot::clientSocket bot_socket) {
    
    // If its a direct message, the bot replies with a random fact 
    if(isDm) {
        bot::send_message("PRIVMSG " + nickname + " :"+get_random_fact()+"\r\n", bot_socket); //FIXME: make random sentence
        return;
    }
    // Responds to any commands beginning with '!'    
    if(text[0] != '!')  {
        return;
    }

    std::string response;
    // Handle the !slap command
    if(text.starts_with("!slap")) {
        slap_command(text, nickname, bot_details, response);
    } 
    else if (text == "!hello") {
        response = "Hey, " + nickname;
    }
    // Extra freature added to allow the user to set a topic in the current channel
    else if(text.starts_with("!topic ")){
        topic_command(text, channel, bot_socket, bot_details, response);
    }

    if(response.empty())
        return;
    bot::send_message("PRIVMSG " + channel + " :"+response+"\r\n", bot_socket);
}