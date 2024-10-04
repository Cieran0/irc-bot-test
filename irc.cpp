#include <irc.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

irc::command irc::parseCommand(const std::string& message) {
    command parsedCommand;
    parsedCommand.raw = message;

    //std::cout << "[\nparsedCommand.raw: " << message << std::endl;

    size_t name_end = message.find_first_of(' ');

    //std::cout << "name_end: " << name_end << std::endl;

    if (name_end == std::string::npos) {
        return parsedCommand; 
    }
    parsedCommand.name = message.substr(0, name_end);

    //std::cout << "parsedCommand.name: " << parsedCommand.name << std::endl;

    size_t after_name = name_end + 1;

    //std::cout << "afer_name: " << after_name << std::endl;

    std::string argsString = message.substr(after_name);

    //std::cout << "argsString: " << argsString << std::endl;

    size_t endOfSpaceSeperated = argsString.find_first_of(':');

    std::string last_args;

    //std::cout << "last_args: " << last_args << std::endl;

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

bool irc::isKnownNumericReply(const std::string& number) {
    if(number.length() != 3)
        return false;

    if(!std::all_of(number.begin(), number.end(), isdigit))
        return false;

    int n = std::stoi(number);

    if(validNumericReplies.find((irc::numeric_reply)n) != validNumericReplies.end())
        return true;

    return false;
}