#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <irc_numberic_replies.hpp>

namespace irc {
    struct command
    {
        std::string raw;
        std::string name;
        std::vector<std::string> arguments;
    };

    irc::command parseCommand(const std::string& message);
    
    bool isKnownNumericReply(const std::string& message);
}