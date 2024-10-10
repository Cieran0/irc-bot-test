#include <bot_new.hpp>

int main(int argc, const char** argv) {
    std::vector<std::string_view> args;

    //provide args vector with command-line arguments

    for (int i = 1; i < argc; i++)
        args.push_back(std::string_view(argv[i]));

    return bot::main(std::string_view(argv[0]), args); //Calls the bot main function with program name and arguments
}

//Global variables to use for the bots state and to track users in the channel
bool bot::isAlive = false;
std::unordered_set<std::string> bot::usersInBotChannel;


//main function for the bot
int bot::main(const std::string_view& program, const std::vector<std::string_view>& arguments) {
    
    //get the bots details 
    bot::details botDetails = bot::getDetailsFromArguments(arguments);
    
    //Open the socket connection to server
    bot::clientSocket botSocket = bot::openSocket(botDetails);

    bot::isAlive = true;
    bot::sendInitalMessages(botSocket, botDetails); //send initial IRC messages to the server 

    //main loop to read and handle the messages on the server
    while (bot::isAlive){
        std::string message = bot::readMessage(botSocket);

        if (message.empty()){
            continue; //Should only happen if !bot::isAlive
        }

        irc::command recievedCommand = irc::parseCommand(message); //Parse the message recieved into an IRC command
        
        //Handle failed parsing 
        if(recievedCommand.name.empty()) {
            std::cerr << "Failed to parse: [" << message << "]" << std::endl;
            return -10;
        }

        //Handle recieved command
        bot::handleCommand(recievedCommand, botDetails, botSocket);

    }
    
    return 0; //exits the program
}

//Function that recieves details from the command-line arguments
bot::details bot::getDetailsFromArguments(const std::vector<std::string_view>& arguments) {

    bot::details botDetails; //Acts as a botDetails object that stores details of the bot
    botDetails.channel = "#"; //Default channel
    botDetails.ip = "::1"; //Default IP
    botDetails.name = "slap_bot"; // Default name 
    botDetails.port = "6667"; // Default port for the IRC 

    // Process command line arguments using pairs (the option and then its value)
    for (size_t i = 0; i < arguments.size(); i+=2)
    {
        if(arguments.size() < i+2){
            std::cerr << "Invalid amount of args: " << arguments.size() << std::endl; // Error if given incomplete pair
            exit(-1); // Exit the program
        }

        // handle the hosst argument
        else if(arguments[i] == "--host") {
            botDetails.ip = arguments[i+1];
            
        } 

        // Handle port arguments 
        else if (arguments[i] == "--port")
        {
           
            std::string_view port = arguments[i + 1];

            // Checks if next argument is another option, suggesting no value was given
            if (arguments[i+1].substr(0,2) == "--")
            {
                std::cout << "No port number provided. " << std:: endl;
                exit(-1);
            }
            
            // Searches a value given after an option to make sure there is no letters in the port number 
            for (char c : port) {
                if (!std::isdigit(c)) {  
                    std::cerr << "Error: Invalid port number. Port must be a number." << std::endl;
                    exit(-1);  
                }
            }

            botDetails.port = port;
        }

        // Handle name argument
        else if (arguments[i] == "--name")
        {

            // Checks if next argument is another option, suggesting no value was given
            if (arguments[i+1].substr(0, 2) == "--") {
            std::cout << "Error: No name provided after --name. " << std::endl;
            exit(-1);  
            }
            
            // Checks after a value to make sure that the next argument is another option.
            //If its not another option then the user has put a space in their bot name 
            else if (arguments[i+2].substr(0, 2) != "--") {
            std::cout << "Error: There are spaces in your name. " << std::endl;
            exit(-1);  
            }

            botDetails.name = arguments[i+1];
        }

        // Handles the channel argument
        else if (arguments[i] == "--channel")
        {
            botDetails.channel = arguments[i+1];

            // Ensures teh channel name is formatted with a '#' at the start
            if(botDetails.channel[0] != '#'){
                std::cout << "Channel name formatted incorrectly. Include # before channel names. " << std::endl;
                exit(-1);
            }
        }
        
        // Handles unknown args
        else {
            std::cerr << "Invalid arg " << arguments[i];
            exit(-1);
        }
    }
    
    return botDetails; // Returns the bot details after they're populated
}

// Function to open a socket connection to the IRC server
bot::clientSocket bot::openSocket(const bot::details& botDetails) {
    bot::clientSocket botSocks = -1; // Initialize the socket

#ifdef _WIN32
    // Windows specific socket initialization
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return botSocks; // Return failure
    }
#endif

    int status;
    struct sockaddr_in6 serv_addr; // Structure for IPv6 addresses

    // Create the socket
    if ((botSocks = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        clean_up(); 
        return botSocks; // Returns a failure 
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); // Clears the previously created struct
    serv_addr.sin6_family = AF_INET6; // Sets to IPv6 to avoid IPv4 default
    serv_addr.sin6_port = htons(std::stoi(std::string(botDetails.port))); // Sets the port

    // Converts IP address from text to binary 
    if (inet_pton(AF_INET6, std::string(botDetails.ip).c_str(), &serv_addr.sin6_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl; 
        clean_up(); 
        return botSocks; 
    }
    std::cout << "Connecting..." << std::endl; 

    if ((status = connect(botSocks, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(botSocks); 
        clean_up(); 
        return botSocks;
    }
    std::cout << "Connected!" << std::endl; // Print statement for a successful connection


    return botSocks;// Return the valid socket
}

// Queue to store recieved messages from the server 
std::queue<std::string> readQueue;
 
// Funciton to read a message from the socket  
std::string bot::readMessage(bot::clientSocket botSocket) {

    // Check to see if there are messages currently stored in the queue
    if(!readQueue.empty()) {
        std::string out = readQueue.front(); // Get the top message of the queue
        readQueue.pop(); // Remove it from the queue 
        return out; // Return the message 
    }
    
    char buffer[1024] = { 0 }; // Buffer to store the message 
    int valread = recv(botSocket, buffer, 1024 -1, 0); // Read from the socket 
        
    // Check for errors of disconnection from the server    
    if (valread <= 0 )
    {
        bot::die(); // Terminate the bot
        return std::string(); // Return an empty string 
    }  
    
    std::string messages = std::string(buffer); // Convert the buffer to a string 

    // Split the messages and pass them to the readQueue
    for(const std::string& s: split_string(messages,"\r\n",false)) {
        readQueue.push(s);
    }

    // Return the first message from the queue
    if(!readQueue.empty()) {
        std::string out = readQueue.front();
        readQueue.pop();
        return out;
    }
    return std::string(); // Return an empty string if there is no message
}  

// Function to get a random user to slap
std::string bot::getRandomUser(const std::string& sender, bot::details botInfo) {
    std::vector<std::string> eligibleUsers; // Vector to store a valid user for slapping 

    // Populate valid users with the set of users in the channel
    for (const std::string& user : bot::usersInBotChannel) {
        if (user != sender && user != botInfo.name) {
            eligibleUsers.push_back(user);
        }
    }

    // If there are not enough users in the server to slap, an empty string is returned
    if (eligibleUsers.empty()) {
        return std::string();
    }

    // First eligible user is returned to be slapped
    return eligibleUsers[0];
}

// function that allows a message to be sent to the IRC server 
void bot::sendMessage(std::string message, bot::clientSocket botSocket) {
    // Checks to see if bot is alive first 
    if (bot::isAlive){
        // Uses the socket to send a message 
        send(botSocket, message.c_str(),message.length(), 0);
    }
}  

// Funciton to send the IRC connection message for the bot 
void bot::sendInitalMessages(bot::clientSocket botSocks, bot::details botDetails) {
    std::string initialMessage1 = "NICK " + std::string(botDetails.name) + "\r\n";
    std::string initialMessage2 = "USER " + std::string(botDetails.name) + " 0 * :"+std::string(botDetails.name)+"\r\n";
    std::string initialMessage3 = "JOIN " + std::string(botDetails.channel) + "\r\n";

    bot::sendMessage(initialMessage1,botSocks);
    bot::sendMessage(initialMessage2,botSocks);
    bot::sendMessage(initialMessage3,botSocks);

}

// Function that terminates the bots connection
void bot::die() {
    std::cout << "Disconnect from server" << std::endl;
    bot::isAlive = false;
}

// Function to process the commands read from the server 
void bot::handleServerCommand(std::string hostname, std::vector<std::string> arguments) {

    irc::numeric_reply numericReply;

    // If the reply is unknown, error is logged and the bot is exited
    if(!irc::isKnownNumericReply(arguments[0])) {
        std::cout << "UKNKOWN" << arguments[0] << std::endl;
        exit(-1);
    }

    numericReply = (irc::numeric_reply)std::stoi(arguments[0]);

    // Switch on the recognized numeric reply to handle different server responses
    switch (numericReply) {
        case irc::RPL_WELCOME:
        case irc::RPL_YOURHOST:
        case irc::RPL_NOTOPIC:
        case irc::RPL_CREATED:
        case irc::RPL_MYINFO:
        case irc::ERR_NOMOTD:
        case irc::RPL_LUSERCLIENT:

            //print this
            break;
        case irc::RPL_NAMREPLY: {
            std::string namesRaw = arguments.back(); // Extract the raw names string 
            std::vector<std::string> names = split_string(namesRaw, " ", true); // Separate names by a space
            for(const std::string& name : names) {
                bot::usersInBotChannel.insert(name); // Add each user to the bot's tracking set
            }
            break;
        }
        case irc::RPL_ENDOFNAMES:
            //ignore, this message indecats the end of the name list
            break;
        default:
            std::cout << "Unhandled: " << irc::validNumericReplies.find(numericReply)->second << std::endl;
            break;
    }
}


// Function that generates a random fact or sentence 
std::string getRandomSentence() {
    
    std::vector<std::string> facts; 

    std::ifstream randomFacts("randomFacts.txt");

    if (!randomFacts)
    {
        std::cout << "Unable to open the file" << std::endl;
        return "Error: unable to retrieve fact ";
    }
    
    std::string line;

    while (std::getline(randomFacts, line))
    {
        facts.push_back(line);
    }

    randomFacts.close();
    
    if (facts.empty()){
        return "No facts found in the file. ";
    }

    std::srand(std::time(0));

    int randomIndex = std::rand() % facts.size();

    return facts[randomIndex];
}

// Funcion to handle responses to private (i.e., PRIVMSG messages) messages from the user 
void bot::respondToPrivmsg(std::string nickname, std::string channel, std::string text, bool isDm, bot::details botDetails, bot::clientSocket botSocket) {
    
    // If its a direct message, the bot replies with a random fact 
    if(isDm) {
        bot::sendMessage("PRIVMSG " + nickname + " :"+getRandomSentence()+"\r\n", botSocket); //FIXME: make random sentence
        return;
    }

    // Responds to any commands beginning with '!'    
    if(text[0] == '!') {
        std::string response;

        // Handle the !slap command without specifying a user
        if(text == "!slap") {
            std::string user = getRandomUser(nickname, botDetails); // Get a user to slap 
            if(user.empty()) {
                response = "There's no one to slap!";
            } else {
                response = "*Slapped " + user + " with a trout*";
            }
        } 

        // Handle !slap command with a specified target 
        else if (text.starts_with("!slap ")) {
            std::string user = split_string(text, " ", true)[1];
            if(bot::usersInBotChannel.find(user) != bot::usersInBotChannel.end()) {
                response = "*Slapped " + user + " with a trout*";
            } else {
                response = "*Slapped " + nickname + " with a trout* since " + user + " couldn't be found!";
            }
        } else if (text == "!hello") {
            response = "Hey, " + nickname;
        }

        // Extra freature added to allow the user to set a topic in the current channel
        else if(text.starts_with("!topic ")){
            std::string setTopic;

            setTopic = text.substr(7);
            bot::sendMessage("TOPIC " + channel + " :" + setTopic +"\r\n", botSocket);
            bot::readMessage(botSocket);
            bot::sendMessage("NAMES " + channel + "\r\n", botSocket);
            irc::command recievedCommand = irc::parseCommand(bot::readMessage(botSocket)); //Parse the message recieved into an IRC command

            //Handle recieved command
            bot::handleCommand(recievedCommand, botDetails, botSocket);

            response = "users who can engage with topic " + setTopic + ": ";
            for (const std::string& name : bot::usersInBotChannel)
            {
                response += name + ", " ;
            }
            
        }
 
        
        if(response.empty())
            return;
        bot::sendMessage("PRIVMSG " + channel + " :"+response+"\r\n", botSocket);
    }

}

// Function that handles a users command 
void bot::handleUserCommand(std::string nickname, std::string username, std::string ip, std::vector<std::string> arguments, bot::clientSocket botSocket, bot::details botDetails) {

    std::string channel = arguments[1];

    std::cout << "arguments[0]: " << arguments[0] << std::endl;
    
    // When PRIVMSG is seen respontToPrivmsg() is called and the appropriate parameters are given to it
    if(arguments[0] == "PRIVMSG") 
    {
        std::string text = arguments[2];
        bool isDm = (channel == botDetails.name);
        respondToPrivmsg(nickname, channel, text, isDm, botDetails, botSocket);
    }

    // If JOIN is seen the bot will print the user and what channel they have joined 
    // The user is also added to the usersInBotChannel
    else if (arguments[0] == "JOIN") 
    {
        std::cout << nickname << " joined channel: [" << channel << "]" << std::endl;
        bot::usersInBotChannel.insert(nickname);
    } 

    // If PART is seen then a print statement is produced specifying the user and what channel they have left
    // The specific user is also removed from the usersInBotChannel 
    else if (arguments[0] == "PART") 
    {

        std::cout << nickname << " left channel: [" << channel << "]" << std::endl;
        if(nickname != botDetails.name) {
            bot::usersInBotChannel.erase(nickname);
        }
    } 

    // When QUIT is then a print statement is produced specifying the user who has left the server
    else if (arguments[0] == "QUIT") 
    {
        std::cout << nickname << " quit with message : [" << arguments[1] << "]" << std::endl;
        if(nickname != "stap_bot2") {
            bot::usersInBotChannel.erase(nickname);
        }
    }

    // Else statement that handles if an unknown command is read from the server
    else 
    {
        std::cerr << "UKNOWN COMMAND " << arguments[0] << std::endl;
    }
}

void bot::handleCommand(irc::command commandToHandle, bot::details botDetails, bot::clientSocket botSocket ) {

    // If command starts with colon, it's either from user or server
    if(commandToHandle.name.starts_with(":")) 
    {

        // Extract the user or hostname 
        std::string hostnameOrUserNickname = commandToHandle.name.substr(1,commandToHandle.name.length()-1);
        size_t endOfNickname = hostnameOrUserNickname.find('!'); // find nickname delimiter

        // If '!' exists, its from a user 
        if(endOfNickname != std::string::npos)
        {
            // Raw string is extracted, containing nickname, userrname and the IP 
            std::string userRaw = hostnameOrUserNickname;
            size_t endOfUserName = userRaw.find('@'); // indicates the end of the username
            std::string nickname = userRaw.substr(0,endOfNickname); // Fimd the nickname
            std::string username = userRaw.substr(endOfNickname+1, endOfUserName-endOfNickname-1); // Gets the username
            std::string ip = userRaw.substr(endOfUserName+1); // Get the IP address

            // Handle teh user command using the extracted information
            handleUserCommand(nickname,username,ip, commandToHandle.arguments, botSocket, botDetails);
        } 

        // Otherwise it is established as a server message
        else 
        {
            std::string hostname = hostnameOrUserNickname;
            handleServerCommand(hostname, commandToHandle.arguments);
        }
    } 

    // Checks if a command is a PING command
    else if (commandToHandle.name == "PING") 
    {
        std::string raw = commandToHandle.raw;
        raw[1] = 'O';
        raw += "\r\n";
        bot::sendMessage(raw, botSocket);
    } 
    else 
    {
        std::cout << "Uknown command: "<< commandToHandle.name << std::endl;
    }
}