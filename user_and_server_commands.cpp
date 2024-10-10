#include <bot.hpp>

/*
    Handles commands from the server. e.g. :HOSTNAME 001 bot_name :Hi, welcome to IRC\r\n
*/
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
            //We can ingore all of these
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
            //ignore, this message indecates the end of the name list
            break;
        default:
            break;
    }
}

/*
    Handles commands from the users e.g. :nick!user@ip PRIVMSG #channel :message\r\n
*/
void bot::handle_user_command(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments, bot::clientSocket bot_socket, bot::details bot_details) {

    std::string channel = arguments[1];
    
    // When PRIVMSG is seen respontToPrivmsg() is called and the appropriate parameters are given to it
    if(arguments[0] == "PRIVMSG") {
        std::string text = arguments[2];
        bool isDm = (channel == bot_details.name);
        respond_to_private_message(nickname, channel, text, isDm, bot_details, bot_socket);
    }

    // If JOIN is seen the bot will print the user and what channel they have joined 
    // The user is also added to the users_in_bot_channel
    else if (arguments[0] == "JOIN") {
        std::cout << nickname << " joined channel: [" << channel << "]" << std::endl;
        bot::users_in_bot_channel.insert(nickname);
    } 

    // If PART is seen then a print statement is produced specifying the user and what channel they have left
    // The specific user is also removed from the users_in_bot_channel 
    else if (arguments[0] == "PART") {
        std::cout << nickname << " left channel: [" << channel << "]" << std::endl;
        if(nickname != bot_details.name) {
            bot::users_in_bot_channel.erase(nickname);
        }
    } 

    // When QUIT is then a print statement is produced specifying the user who has left the server
    else if (arguments[0] == "QUIT") {
        std::cout << nickname << " quit with message : [" << arguments[1] << "]" << std::endl;
        if(nickname != "stap_bot2") {
            bot::users_in_bot_channel.erase(nickname);
        }
    }

    // Else statement that handles if an unknown command is read from the server
    else {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
    }
}

/*
    Decides if command is from user or from server, handling them accordingly.
*/
void bot::handle_command(irc::command command_to_handle, bot::details bot_details, bot::clientSocket bot_socket ) {

    // If command starts with colon, it's either from user or server
    if(command_to_handle.name.starts_with(":")) {

        // Extract the user or hostname 
        std::string hostname_or_nickname = command_to_handle.name.substr(1,command_to_handle.name.length()-1);
        size_t end_of_nickname = hostname_or_nickname.find('!'); // find nickname delimiter

        // If '!' exists, its from a user 
        if(end_of_nickname != std::string::npos) {
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
        else  {
            std::string hostname = hostname_or_nickname;
            handle_server_command(hostname, command_to_handle.arguments);
        }
    } 

    // Checks if a command is a PING command
    else if (command_to_handle.name == "PING") {
        std::string raw = command_to_handle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::send_message(raw, bot_socket);
    } 
    else {
        std::cerr << "Uknown command: "<< command_to_handle.name << std::endl;
    }
}