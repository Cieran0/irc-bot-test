#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <irc_numberic_replies.hpp>

namespace irc {
    struct command
    {
        // raw for entire message unparsed I assume 
        std::string raw;
        // Name used for storing a users name
        std::string name;
        // arguments used for arguments in an IRC message
        std::vector<std::string> arguments;
    };

    irc::command parseCommand(const std::string& message);
    
    bool isKnownNumericReply(const std::string& message);
}