#include <bot.hpp>

/*
    Handles !slap
*/
void bot::slap_command(std::string &text, std::string &nickname, const bot::details &bot_details, std::string &response)
{
    //! slap with no target
    if (text == "!slap") {
        std::string user = get_random_user(nickname, bot_details); // Get a user to slap
        if (user.empty()) {
            response = "There's no one to slap!";
        }
        else {
            response = "*Slapped " + user + " with a trout*";
        }
    }
    else {
        // Handle !slap command with a specified target
        std::string user = split_string(text, " ", true)[1];
        if (bot::users_in_bot_channel.find(user) != bot::users_in_bot_channel.end()) {
            response = "*Slapped " + user + " with a trout*";
        }
        else {
            response = "*Slapped " + nickname + " with a trout* since " + user + " couldn't be found!";
        }
    }
}

/*
    Handles !topic
*/
void bot::topic_command(std::string &text, std::string &channel, bot::clientSocket bot_socket, const bot::details &bot_details, std::string &response)
{
    std::string setTopic = text.substr(7);
    bot::send_message("TOPIC " + channel + " :" + setTopic + "\r\n", bot_socket);
    bot::read_message(bot_socket); //Ignore topic response
    bot::send_message("NAMES " + channel + "\r\n", bot_socket);

    irc::command recieved_command = irc::parse_command(bot::read_message(bot_socket)); // Get the names from the server
    
    // Sets users_in_bot_channel to the names got
    bot::handle_command(recieved_command, bot_details, bot_socket);
    
    response = "users who can engage with topic " + setTopic + ":";
    for (const std::string &name : bot::users_in_bot_channel) {
        response += " " + name + ",";
    }
    response.pop_back(); //Remove final ','
}