#include <bot_new.hpp>

int main(int argc, char** argv) {
    std::vector<std::string_view> args;

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args);
}


bool bot::isAlive = false;
/*
    Avoid C style main
*/
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    bot::details botDetails = bot::getDetailsFromArguments(arguments);

    bot::socket botSocket = bot::openSocket(botDetails);

    bot::handleSocket(botSocket);

    while(bot::isAlive) {
        std::string message = bot::getNextMessage();
        bot::handleMessage(message);
    }

    return 0;
}


bot::details bot::getDetailsFromArguments(const std::vector<std::string_view>& arguments) {

    //TODO: parse arguments
    bot::details botDetails;
    botDetails.channel = "#";
    botDetails.ip = "::1";
    botDetails.name = "slap_bot";
    botDetails.port = "6667";

    return botDetails;
}

bot::socket bot::openSocket(const bot::details& botDetails) {

    bot::socket socket = 0;

    //TODO: connect to server

    return socket;
}

void bot::handleSocket(bot::socket botSocket) {

}   

/*IDK*/
void bot::sendInitalMessages(/*args*/) {

}

/*IDK*/
std::string bot::getNextMessage(/*args*/) {
    return std::string("message");
}

/*IDK*/
void bot::handleMessage(const std::string& message/*args*/) {

}

/*IDK*/
void bot::sendMessage(/*args*/) {

}

