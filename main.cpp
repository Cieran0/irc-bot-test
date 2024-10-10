#include <bot.hpp>

int main(int argc, const char** argv) {
    std::vector<std::string_view> args;

    //provide args vector with command-line arguments

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args); //Calls the bot main function with program name and arguments
}

//Global variables to use for the bots state and to track users in the channel
bool bot::is_alive = false;
std::unordered_set<std::string> bot::users_in_bot_channel;

//main function for the bot
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //get the bots details 
    bot::details bot_details = bot::get_details_from_arguments(arguments);
    
    //Open the socket connection to server
    bot::clientSocket bot_socket = bot::open_socket(bot_details);

    bot::is_alive = true;
    bot::send_inital_message(bot_socket, bot_details); //send initial IRC messages to the server 

    //main loop to read and handle the messages on the server
    while (bot::is_alive){
        std::string message = bot::read_message(bot_socket);

        if (message.empty()){
            continue; //Should only happen if !bot::is_alive
        }

        irc::command recieved_command = irc::parse_command(message); //Parse the message recieved into an IRC command
        
        //Handle failed parsing 
        if(recieved_command.name.empty()) {
            std::cerr << "Failed to parse: [" << message << "]" << std::endl;
            return -1;
        }

        //Handle recieved command
        bot::handle_command(recieved_command, bot_details, bot_socket);

    }
    
    return 0; //exits the program
}

//Function that recieves details from the command-line arguments
bot::details bot::get_details_from_arguments(const std::vector<std::string_view>& arguments) {

    bot::details bot_details; //Acts as a bot_details object that stores details of the bot
    bot_details.channel = "#"; //Default channel
    bot_details.ip = "::1"; //Default IP
    bot_details.name = "slap_bot"; // Default name 
    bot_details.port = "6667"; // Default port for the IRC 

    // Process command line arguments using pairs (the option and then its value)
    for (size_t i = 0; i < arguments.size(); i+=2)
    {
        if(arguments.size() < i+2){
            std::cerr << "Invalid amount of args: " << arguments.size() << std::endl; // Error if given incomplete pair
            exit(-1); // Exit the program
        }

        // handle the hosst argument
        else if(arguments[i] == "--host") {
            bot_details.ip = arguments[i+1];
            
        } 

        // Handle port arguments 
        else if (arguments[i] == "--port")
        {
           
            std::string_view port = arguments[i + 1];

            // Checks if next argument is another option, suggesting no value was given
            if (arguments[i+1].substr(0,2) == "--")
            {
                std::cerr << "No port number provided. " << std:: endl;
                exit(-1);
            }
            
            // Searches a value given after an option to make sure there is no letters in the port number 
            for (char c : port) {
                if (!std::isdigit(c)) {  
                    std::cerr << "Error: Invalid port number. Port must be a number." << std::endl;
                    exit(-1);  
                }
            }

            bot_details.port = port;
        }

        // Handle name argument
        else if (arguments[i] == "--name")
        {

            // Checks if next argument is another option, suggesting no value was given
            if (arguments[i+1].substr(0, 2) == "--") {
            std::cerr << "Error: No name provided after --name. " << std::endl;
            exit(-1);  
            }
            
            // Checks after a value to make sure that the next argument is another option.
            //If its not another option then the user has put a space in their bot name 
            else if (arguments[i+2].substr(0, 2) != "--") {
            std::cerr << "Error: There are spaces in your name. " << std::endl;
            exit(-1);  
            }

            bot_details.name = arguments[i+1];
        }

        // Handles the channel argument
        else if (arguments[i] == "--channel")
        {
            bot_details.channel = arguments[i+1];

            // Ensures teh channel name is formatted with a '#' at the start
            if(bot_details.channel[0] != '#'){
                std::cerr << "Channel name formatted incorrectly. Include # before channel names. " << std::endl;
                exit(-1);
            }
        }
        
        // Handles unknown args
        else {
            std::cerr << "Invalid arg " << arguments[i];
            exit(-1);
        }
    }
    
    return bot_details; // Returns the bot details after they're populated
}

// Function to open a socket connection to the IRC server
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
 
// Funciton to read a message from the socket  
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

// Function to get a random user to slap
std::string bot::get_random_user(const std::string& sender, bot::details botInfo) {
    std::vector<std::string> eligible_users; // Vector to store a valid user for slapping 

    // Populate valid users with the set of users in the channel
    for (const std::string& user : bot::users_in_bot_channel) {
        if (user != sender && user != botInfo.name) {
            eligible_users.push_back(user);
        }
    }

    // If there are not enough users in the server to slap, an empty string is returned
    if (eligible_users.empty()) {
        return std::string();
    }

    // First eligible user is returned to be slapped
    return eligible_users[0];
}

// function that allows a message to be sent to the IRC server 
void bot::send_message(std::string message, bot::clientSocket bot_socket) {
    // Checks to see if bot is alive first 
    if (bot::is_alive){
        // Uses the socket to send a message 
        send(bot_socket, message.c_str(),message.length(), 0);
    }
}  

// Funciton to send the IRC connection message for the bot 
void bot::send_inital_message(bot::clientSocket bot_socket, bot::details bot_details) {
    std::string nick_message = "NICK " + std::string(bot_details.name) + "\r\n";
    std::string user_message = "USER " + std::string(bot_details.name) + " 0 * :"+std::string(bot_details.name)+"\r\n";
    std::string join_message = "JOIN " + std::string(bot_details.channel) + "\r\n";

    bot::send_message(nick_message,bot_socket);
    bot::send_message(user_message,bot_socket);
    bot::send_message(join_message,bot_socket);

}

// Function that terminates the bots connection
void bot::die() {
    std::cerr << "Disconnect from server" << std::endl;
    bot::is_alive = false;
}

// Function to process the commands read from the server 
void bot::handle_server_command(std::string hostname, std::vector<std::string> arguments) {

    irc::numeric_reply numeric_reply;

    // If the reply is unknown, error is logged and the bot is exited
    if(!irc::is_known_numeric_reply(arguments[0])) {
        std::cerr << "UKNKOWN" << arguments[0] << std::endl;
    }

    numeric_reply = (irc::numeric_reply)std::stoi(arguments[0]);

    // Switch on the recognized numeric reply to handle different server responses
    switch (numeric_reply) {
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
            std::string names_raw = arguments.back(); // Extract the raw names string 
            std::vector<std::string> names = split_string(names_raw, " ", true); // Separate names by a space
            for(const std::string& name : names) {
                bot::users_in_bot_channel.insert(name); // Add each user to the bot's tracking set
            }
            break;
        }
        case irc::RPL_ENDOFNAMES:
            //ignore, this message indecats the end of the name list
            break;
        default:
            std::cerr << "Unhandled: " << irc::valid_numeric_replies.find(numeric_reply)->second << std::endl;
            break;
    }
}


// Function that generates a random fact or sentence 
std::string get_random_fact() {
    
    std::vector<std::string> facts; 

    std::ifstream random_facts("random_facts.txt");

    if (!random_facts)
    {
        std::cerr << "Unable to open the file" << std::endl;
        return "Error: unable to retrieve fact ";
    }
    
    std::string line;

    while (std::getline(random_facts, line))
    {
        facts.push_back(line);
    }

    random_facts.close();
    
    if (facts.empty()){
        return "No facts found in the file. ";
    }

    std::srand(std::time(0));

    int random_index = std::rand() % facts.size();

    return facts[random_index];
}

// Funcion to handle responses to private (i.e., PRIVMSG messages) messages from the user 
void bot::respond_to_private_message(std::string nickname, std::string channel, std::string text, bool isDm, bot::details bot_details, bot::clientSocket bot_socket) {
    
    // If its a direct message, the bot replies with a random fact 
    if(isDm) {
        bot::send_message("PRIVMSG " + nickname + " :"+get_random_fact()+"\r\n", bot_socket); //FIXME: make random sentence
        return;
    }

    // Responds to any commands beginning with '!'    
    if(text[0] == '!') {
        std::string response;

        // Handle the !slap command without specifying a user
        if(text == "!slap") {
            std::string user = get_random_user(nickname, bot_details); // Get a user to slap 
            if(user.empty()) {
                response = "There's no one to slap!";
            } else {
                response = "*Slapped " + user + " with a trout*";
            }
        } 

        // Handle !slap command with a specified target 
        else if (text.starts_with("!slap ")) {
            std::string user = split_string(text, " ", true)[1];
            if(bot::users_in_bot_channel.find(user) != bot::users_in_bot_channel.end()) {
                response = "*Slapped " + user + " with a trout*";
            } else {
                response = "*Slapped " + nickname + " with a trout* since " + user + " couldn't be found!";
            }
        } else if (text == "!hello") {
            response = "Hey, " + nickname;
        }

        // Extra freature added to allow the user to set a topic in the current channel
        else if(text.starts_with("!topic ")){
            std::string setTopic;

            setTopic = text.substr(7);
            bot::send_message("TOPIC " + channel + " :" + setTopic +"\r\n", bot_socket);
            bot::read_message(bot_socket);
            bot::send_message("NAMES " + channel + "\r\n", bot_socket);
            irc::command recieved_command = irc::parse_command(bot::read_message(bot_socket)); //Parse the message recieved into an IRC command

            //Handle recieved command
            bot::handle_command(recieved_command, bot_details, bot_socket);

            response = "users who can engage with topic " + setTopic + ": ";
            for (const std::string& name : bot::users_in_bot_channel)
            {
                response += name + ", " ;
            }
            
        }
 
        
        if(response.empty())
            return;
        bot::send_message("PRIVMSG " + channel + " :"+response+"\r\n", bot_socket);
    }

}

// Function that handles a users command 
void bot::handle_user_command(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments, bot::clientSocket bot_socket, bot::details bot_details) {

    std::string channel = arguments[1];
    
    // When PRIVMSG is seen respontToPrivmsg() is called and the appropriate parameters are given to it
    if(arguments[0] == "PRIVMSG") 
    {
        std::string text = arguments[2];
        bool isDm = (channel == bot_details.name);
        respond_to_private_message(nickname, channel, text, isDm, bot_details, bot_socket);
    }

    // If JOIN is seen the bot will print the user and what channel they have joined 
    // The user is also added to the users_in_bot_channel
    else if (arguments[0] == "JOIN") 
    {
        std::cout << nickname << " joined channel: [" << channel << "]" << std::endl;
        bot::users_in_bot_channel.insert(nickname);
    } 

    // If PART is seen then a print statement is produced specifying the user and what channel they have left
    // The specific user is also removed from the users_in_bot_channel 
    else if (arguments[0] == "PART") 
    {

        std::cout << nickname << " left channel: [" << channel << "]" << std::endl;
        if(nickname != bot_details.name) {
            bot::users_in_bot_channel.erase(nickname);
        }
    } 

    // When QUIT is then a print statement is produced specifying the user who has left the server
    else if (arguments[0] == "QUIT") 
    {
        std::cout << nickname << " quit with message : [" << arguments[1] << "]" << std::endl;
        if(nickname != "stap_bot2") {
            bot::users_in_bot_channel.erase(nickname);
        }
    }

    // Else statement that handles if an unknown command is read from the server
    else 
    {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
    }
}

void bot::handle_command(irc::command command_to_handle, bot::details bot_details, bot::clientSocket bot_socket ) {

    // If command starts with colon, it's either from user or server
    if(command_to_handle.name.starts_with(":")) 
    {

        // Extract the user or hostname 
        std::string hostname_or_nickname = command_to_handle.name.substr(1,command_to_handle.name.length()-1);
        size_t end_of_nickname = hostname_or_nickname.find('!'); // find nickname delimiter

        // If '!' exists, its from a user 
        if(end_of_nickname != std::string::npos)
        {
            // Raw string is extracted, containing nickname, userrname and the IP 
            std::string user_raw = hostname_or_nickname;
            size_t end_of_username = user_raw.find('@'); // indicates the end of the username
            std::string nickname = user_raw.substr(0,end_of_nickname); // Fimd the nickname
            std::string username = user_raw.substr(end_of_nickname+1, end_of_username-end_of_nickname-1); // Gets the username
            std::string ip = user_raw.substr(end_of_username+1); // Get the IP address

            // Handle teh user command using the extracted information
            handle_user_command(nickname,username,ip, command_to_handle.arguments, bot_socket, bot_details);
        } 

        // Otherwise it is established as a server message
        else 
        {
            std::string hostname = hostname_or_nickname;
            handle_server_command(hostname, command_to_handle.arguments);
        }
    } 

    // Checks if a command is a PING command
    else if (command_to_handle.name == "PING") 
    {
        std::string raw = command_to_handle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::send_message(raw, bot_socket);
    } 
    else 
    {
        std::cerr << "Uknown command: "<< command_to_handle.name << std::endl;
    }
}