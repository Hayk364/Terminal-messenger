//g++ -std=c++17 -o messenger main.cpp backend.cpp -lcurl

#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <atomic>
#include <chrono>
#include <cctype>

#ifdef _WIN32
    #define CLEAR_COMMAND "cls"
#else
    #define CLEAR_COMMAND "clear"
#endif

#include "backend.hpp"

std::atomic<bool> running{true};
std::mutex coutMutex;

void ChatUpdater(const std::string& username, const std::string& recipient) {
    while (running) {
        system(CLEAR_COMMAND);
        
        auto chat = Backend::GetChat(username, recipient);
        for (const auto& [sendername, message] : chat) {
            if (sendername == username) {
                std::cout << "me> " << message << std::endl;
            } else {
                std::cout << sendername << "> " << message << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void InputHandler(const std::string& username, const std::string& recipient) {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "me> ";
        }
        std::string newMessage;
        std::getline(std::cin, newMessage);
        
        if (newMessage == "/exit") {
            running = false;
            break;
        }
        
        if (!newMessage.empty()) {
            bool send = Backend::SendMessage(username, recipient, newMessage);
            if (!send) {
                std::cout << "Failed to send message" << std::endl;
            }
        }
    }
}

int main() {
    bool app = true;
    while (app) {
        std::cout << "Login: l - Register: r -- ";
        char auth;
        std::cin >> auth;
        std::cin.ignore();

        if(auth == 'l'){
            std::cout << "Username: ";
            std::string username;
            std::getline(std::cin, username);
            
            std::cout << "Password: ";
            std::string password;
            std::getline(std::cin, password);
            
            bool login = Backend::Login(username, password);
            if(login) {
                std::map<int,std::string> array = Backend::GetUsers(username);
                system(CLEAR_COMMAND);
                std::cout << "Settings: s" << std::endl;
                for(auto& user : array){
                    std::cout << user.first << " : " << user.second << std::endl;
                }
                std::cout << "... - ";
                char home;
                std::cin >> home;
                std::cin.ignore();
                
                if(isdigit(home)){
                    int index = home - '0';
                    if (array.find(index) != array.end()) {
                        std::string recipient = array[index];

                        running = true;
                        std::thread updater(ChatUpdater, username, recipient);
                        std::thread input(InputHandler, username, recipient);

                        input.join();
                        running = false;
                        updater.join();
                    }
                } else if(home == 's'){
                    std::cout << "Settings" << std::endl;
                    app = false;
                }
            } else {
                std::cout << "Login failed" << std::endl;
            }
        } else if(auth == 'r'){
            std::cout << "Username: ";
            std::string username;
            std::getline(std::cin, username);
            
            std::cout << "Password: ";
            std::string password;
            std::getline(std::cin, password);
            
            bool reg = Backend::Register(username, password);
            if(reg){
                std::cout << "Registration successful" << std::endl;
            }else{
                std::cout << "Error registering" << std::endl;
            }
            app = false;
        }else if(auth == 'c'){
            app = false;
        }
    }
    return 0;
}

