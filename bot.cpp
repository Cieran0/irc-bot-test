#include <bot.hpp>

int main(int argc, const char** argv) {
    std::vector<std::string_view> args;

    //provide args vector with command-line arguments
    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args); //Calls the bot main function with program name and arguments
}

//Global variables to use for the bots state and to track users in the channel
bool bot::is_alive = true;
std::unordered_set<std::string> bot::users_in_bot_channel;

/*
    Initalises and runs the bot.
*/
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //get the bots details 
    bot::details bot_details = bot::get_details_from_arguments(arguments);
    
    //Open the socket connection to server
    bot::clientSocket bot_socket = bot::open_socket(bot_details);

    //Checks if socket is valid
    if(bot_socket < 0)
        return bot_socket;

    bot::is_alive = true;
    bot::send_inital_message(bot_socket, bot_details); //send initial IRC messages to the server 

    //main loop to read and handle the messages on the server
    while (bot::is_alive){
        std::string message = bot::read_message(bot_socket);

        if (message.empty()){
            continue; //Should only happen if !bot::is_alive
        }

        irc::command recieved_command = irc::parse_command(message); //Parse the message recieved into an IRC command
        
        //Handle failed parsing 
        if(recieved_command.name.empty()) {
            std::cerr << "Failed to parse: [" << message << "]" << std::endl;
            return -1;
        }

        //Handle recieved command
        bot::handle_command(recieved_command, bot_details, bot_socket);

    }
    
    return 0; //exits the program
}

/*
    Kills the bot.
*/
void bot::die() {
    std::cerr << "Disconnected from server" << std::endl;
    bot::is_alive = false;
}