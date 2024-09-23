#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <util.hpp>

namespace irc {
    struct command
    {
        std::string name;
        std::vector<std::string> arguments;
    };

    irc::command parseCommand(const std::string& message);
    
}