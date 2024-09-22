#include <bot.hpp>
#include <string_view>
#include <algorithm>
#include <sstream>
#include <vector>
#include <util.hpp>
#include <cstdlib>
#include <ctime>

bool is_whitespace(const std::string& s) {
  return std::all_of(s.begin(), s.end(), isspace);
}

std::vector<std::string> get_online_users(bot& slap_bot) {
    std::vector<std::string> online_users;
    slap_bot.send_message("NAMES #\r\n");
    std::string response = slap_bot.get_next_message();

    std::cout << "Users:" << response;
    int start_of_names = response.find_last_of(':');
    std::string users_split_by_spaces = response.substr(start_of_names+1);
    users_split_by_spaces = users_split_by_spaces.substr(0,users_split_by_spaces.length()-2);
    online_users = split_string(users_split_by_spaces, " ", true);

    for(auto user : online_users) {
        std::cout << "User: [" << user << "]" << std::endl;
    }
    

    return online_users;
}

int main() {

    std::string bot_name="slap_bot";
    std::string ip = "::1";
    std::string place = "#";

    std::string random_sentence = "You suck lol";

    bot slap_bot(ip,6667,bot_name,place);
    slap_bot.send_message("NICK slap_bot\r\n");
    slap_bot.send_message("USER slap_bot 0 * :Gamer\r\n");
    slap_bot.send_message("JOIN #\r\n");

    std::string host_name = slap_bot.get_next_message();
    int split = host_name.find_first_of(' ');
    host_name = host_name.substr(1,split-1);
    std::cout << "Host name: " << host_name << std::endl;

    while (slap_bot.alive())
    {
        std::string message = slap_bot.get_next_message();
        if(message.empty()) {
            continue;
        }
        if(message.starts_with("PING")) {
            message[1] = 'O';
            slap_bot.send_message(message); 
        } else if (message.starts_with(":")) {
            int deliminator = message.find_first_of(' ');
            std::string user_details = message.substr(1,deliminator-1);
            std::string command_details = message.substr(deliminator+1);
            deliminator = user_details.find_first_of('!');
            std::string user_name = user_details.substr(0,deliminator);

            deliminator = command_details.find_first_of(' ');
            std::string command = command_details.substr(0, deliminator);
            if(command.compare("PRIVMSG") == 0) {
                std::string message_details = command_details.substr(deliminator+1);
                deliminator = message_details.find_first_of(' ');
                std::string channel = message_details.substr(0,deliminator);
                std::string text = message_details.substr(deliminator+2);
                text.pop_back();
                text.pop_back();

                std::cout << "User: " << user_name << " Said " << text << " in " << channel << std::endl;
                std::string response;
                if(channel.compare(bot_name) == 0) {
                    response = "PRIVMSG "+user_name+" :"+random_sentence+"\r\n";
                    
                } else if(text.compare("!hello") == 0) {
                    response = "PRIVMSG # :Hey there " + user_name + "\r\n";

                } else if(text.starts_with("!slap")) {
                    std::vector<std::string> online_people = get_online_users(slap_bot);
                    if(text.length() == 5 || is_whitespace(text.substr(5))) {
                        //slap rando
                        online_people.erase(std::find(online_people.begin(), online_people.end(), bot_name));
                        online_people.erase(std::find(online_people.begin(), online_people.end(), user_name));

                        if(online_people.empty()) {
                            response = "PRIVMSG # :[Couldn't find any other people] *Slaps " + user_name + " with a trout instead*\r\n";
                        } else {
                            std::string random_user = user_name;
                            std::srand(std::time(nullptr)); // use current time as seed for random generator
                            int index = std::rand() % online_people.size();
                            response = "PRIVMSG # :*Slaps " + online_people[index] + " with a trout*\r\n";
                        }

                    }

                    else if(text[5] == ' ' && text.length() >= 7) {
                        //Slap person
                        std::string other_dude = text.substr(6);
                        if(!contains(online_people, other_dude)) {
                            response = "PRIVMSG # :[Couldn't find "+other_dude+"] *Slaps " + user_name + " with a trout instead*\r\n";
                        } else {
                            response = "PRIVMSG # :*Slaps " + other_dude + " with a trout*\r\n";
                        }
                    }
                } else if (text.starts_with("!echo ")) {
                    response = "PRIVMSG # :" + text.substr(5) + "\r\n";
                }
                if(!response.empty())
                    slap_bot.send_message(response);
            }


        }
    }
    std::cout << "DIED" << std::endl;
    
}