#include <bot.hpp>

/*
    Recieves details from the command-line arguments
*/
bot::details bot::get_details_from_arguments(const std::vector<std::string_view>& arguments) {

    //Sets default value for bot details
    bot::details bot_details = {
        .name="slap_bot",
        .ip="::1",
        .port="6667",
        .channel="#"
    }; 

    // Process command line arguments using pairs (the option and then its value)
    for (size_t i = 0; i < arguments.size(); i+=2) {
        if(arguments.size() < i+2) {
            std::cerr << "Invalid amount of args: " << arguments.size() << std::endl; // Error if given incomplete pair
            exit(-1); // Exit the program
        }
        // handle the hosst argument
        else if(arguments[i] == "--host") {
            bot_details.ip = arguments[i+1];   
        } 
        // Handle port arguments 
        else if (arguments[i] == "--port") {
            get_port(arguments, i, bot_details);
        }
        // Handle name argument
        else if (arguments[i] == "--name") {
            get_name(arguments, i, bot_details);
        }
        // Handles the channel argument
        else if (arguments[i] == "--channel") {
            get_channel(bot_details, arguments, i);
        }
        // Handles unknown args
        else {
            std::cerr << "Invalid arg: " << arguments[i];
            exit(-1);
        }
    }

    return bot_details; // Returns the bot details after they're populated
}

/*
    Get the channel name from arguments
*/
void bot::get_channel(bot::details &bot_details, const std::vector<std::string_view> &arguments, size_t i) {
    bot_details.channel = arguments[i + 1];

    // Ensures teh channel name is formatted with a '#' at the start
    if (bot_details.channel[0] != '#') {
        std::cerr << "Channel name formatted incorrectly. Include # before channel names. " << std::endl;
        exit(-1);
    }
}

/*
    Get the bot name from arguments
*/
void bot::get_name(const std::vector<std::string_view> &arguments, size_t i, bot::details &bot_details) {
    // Checks if next argument is another option, suggesting no value was given
    if (arguments[i + 1].substr(0, 2) == "--") {
        std::cerr << "Error: No name provided after --name. " << std::endl;
        exit(-1);
    }

    // Checks after a value to make sure that the next argument is another option.
    // If its not another option then the user has put a space in their bot name
    else if (arguments[i + 2].substr(0, 2) != "--") {
        std::cerr << "Error: There are spaces in your name. " << std::endl;
        exit(-1);
    }

    bot_details.name = arguments[i + 1];
}

/*
    Get the port from arguments
*/
void bot::get_port(const std::vector<std::string_view> &arguments, size_t i, bot::details &bot_details) {
    std::string_view port = arguments[i + 1];
    // Checks if next argument is another option, suggesting no value was given
    if (arguments[i + 1].substr(0, 2) == "--") {
        std::cerr << "No port number provided. " << std::endl;
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