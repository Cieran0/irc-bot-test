#include <bot.hpp>

/*
    Gets a random user in the bot's channel.
    Excludes the bot and the sender.
*/
std::string bot::get_random_user(const std::string& sender, bot::details botInfo) {

    std::vector<std::string> eligible_users; // Vector to store a valid user for slapping 
    // Populate valid users with the set of users in the channel
    for (const std::string& user : bot::users_in_bot_channel) {
        if (user != sender && user != botInfo.name) {
            eligible_users.push_back(user);
        }
    }

    // If there are not enough users in the server to slap, an empty string is returned
    if (eligible_users.empty()) {
        return std::string();
    }

    // First eligible user is returned to be slapped
    return eligible_users[0];
}

/*
    Generates a random fact from "random_facts.txt"
*/
std::string bot::get_random_fact() {
    
    std::vector<std::string> facts; 
    std::ifstream random_facts("random_facts.txt");
    if (!random_facts) {
        std::cerr << "Unable to open the file" << std::endl;
        return "Error: unable to retrieve fact ";
    }
    
    std::string line;
    while (std::getline(random_facts, line)) {
        facts.push_back(line);
    }
    random_facts.close();

    if (facts.empty()) {
        return "No facts found in the file. ";
    }
    
    //Get random fact from vector
    std::srand(std::time(0));
    int random_index = std::rand() % facts.size();
    return facts[random_index];
}