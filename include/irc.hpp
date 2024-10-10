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

    irc::command parse_command(const std::string& message);
    
    bool is_known_numeric_reply(const std::string& message);
}