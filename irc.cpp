#include <irc.hpp>
#include <sstream>
#include <iostream>

irc::command irc::parseCommand(const std::string& message) {
    size_t name_end = message.find_first_of(' ');
    command parsedCommand;
    

    if (name_end == std::string::npos) {
        return parsedCommand; 
    }
    parsedCommand.name = message.substr(0, name_end);
    // std::cout << "[\n" << parsedCommand.name << std::endl;

    size_t after_name = name_end + 1;
    std::string argsString = message.substr(after_name);
    size_t endOfSpaceSeperated = argsString.find_first_of(':');

    std::string last_args;
    if (endOfSpaceSeperated != std::string::npos) {
        last_args = argsString.substr(endOfSpaceSeperated + 1);
        argsString = argsString.substr(0, endOfSpaceSeperated);
    }

    std::istringstream iss(argsString);
    std::string arg;
    while (iss >> arg) {
        parsedCommand.arguments.push_back(arg);
    }



    if(!last_args.empty()) {
        parsedCommand.arguments.push_back(last_args);
    }

    // std::cout << "Arguments: " << std::endl;
    // for (const std::string& arg : parsedCommand.arguments) {
    //     std::cout << arg << std::endl;
    // }
    
    // std::cout << "]" << std::endl;
    return parsedCommand;
}
