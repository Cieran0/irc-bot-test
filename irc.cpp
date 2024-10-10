#include <irc.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

irc::command irc::parse_command(const std::string& message) {
    command parsed_command;
    parsed_command.raw = message;

    //Name ends at first space
    size_t name_end = message.find_first_of(' ');

    //If name never ends it returns an empty command
    if (name_end == std::string::npos) {
        return parsed_command; 
    }

    parsed_command.name = message.substr(0, name_end);

    size_t after_name = name_end + 1;

    std::string arguments = message.substr(after_name);

    //If it has text e.g :No Topic Set, set it as the last argument
    size_t end_of_space_seperated = arguments.find_first_of(':');

    std::string last_args;

    if (end_of_space_seperated != std::string::npos) {
        last_args = arguments.substr(end_of_space_seperated + 1);
        arguments = arguments.substr(0, end_of_space_seperated);
    }

    //Get arguments seperated by spaces.
    std::istringstream iss(arguments);
    std::string arg;
    while (iss >> arg) {
        parsed_command.arguments.push_back(arg);
    }

    if(!last_args.empty()) {
        parsed_command.arguments.push_back(last_args);
    }

    return parsed_command;
}

/*
    Check if a code is a known irc reply.
    In form XXX where X is a integer.
*/
bool irc::is_known_numeric_reply(const std::string& number) {

    if(number.length() != 3)
        return false;

    if(!std::all_of(number.begin(), number.end(), isdigit))
        return false;

    int n = std::stoi(number);

    if(valid_numeric_replies.find((irc::numeric_reply)n) != valid_numeric_replies.end())
        return true;

    return false;
}