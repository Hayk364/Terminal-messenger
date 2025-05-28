// Compile command example:
// g++ -std=c++17 -o messenger main.cpp backend.cpp -lcurl

#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <atomic>
#include <chrono>
#include <cctype>
#include <mutex>  // Added to use std::mutex

#ifdef _WIN32
    #define CLEAR_COMMAND "cls"   // Windows clear console command
#else
    #define CLEAR_COMMAND "clear" // Unix-based clear console command
#endif

#include "backend.hpp"

// Atomic boolean flag to control when chat threads should run/stop
std::atomic<bool> running{true};

// Mutex to synchronize output to console to avoid garbled prints
std::mutex coutMutex;

/**
 * Thread function that continuously fetches and displays chat messages
 * between 'username' and 'recipient' every 3 seconds.
 */
void ChatUpdater(const std::string& username, const std::string& recipient) {
    while (running) {
        system(CLEAR_COMMAND); // Clear the console for updated chat view

        // Get chat history from backend
        auto chat = Backend::GetChat(username, recipient);

        // Display chat messages
        for (const auto& [sendername, message] : chat) {
            if (sendername == username) {
                std::cout << "me> " << message << std::endl;
            } else {
                std::cout << sendername << "> " << message << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3)); // Wait before refreshing chat
    }
}

/**
 * Thread function to handle user input.
 * Reads messages from user and sends them via Backend.
 * If user types "/exit", it stops the chat.
 */
void InputHandler(const std::string& username, const std::string& recipient) {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(coutMutex); // Lock cout for clean prompt display
            std::cout << "me> ";
        }
        std::string newMessage;
        std::getline(std::cin, newMessage);

        if (newMessage == "/exit") {
            running = false; // Signal to stop chat
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
    bool app = true; // Main app loop flag

    while (app) {
        std::cout << "Login: l - Register: r -- ";
        char auth;
        std::cin >> auth;
        std::cin.ignore(); // Clear input buffer

        if(auth == 'l'){
            // Login flow
            std::cout << "Username: ";
            std::string username;
            std::getline(std::cin, username);

            std::cout << "Password: ";
            std::string password;
            std::getline(std::cin, password);

            // Try logging in via backend
            bool login = Backend::Login(username, password);
            if(login) {
                // Get list of other users to chat with
                std::map<int,std::string> array = Backend::GetUsers(username);

                system(CLEAR_COMMAND);
                std::cout << "Settings: s" << std::endl;

                // Display list of users
                for(auto& user : array){
                    std::cout << user.first << " : " << user.second << std::endl;
                }
                std::cout << "... - ";
                char home;
                std::cin >> home;
                std::cin.ignore();

                // If user chooses a number, start chat with that user
                if(isdigit(home)){
                    int index = home - '0';
                    if (array.find(index) != array.end()) {
                        std::string recipient = array[index];

                        running = true;
                        // Start chat updater and input handler threads
                        std::thread updater(ChatUpdater, username, recipient);
                        std::thread input(InputHandler, username, recipient);

                        input.join();  // Wait for input thread to finish (user typed /exit)
                        running = false;
                        updater.join(); // Then wait for updater thread to stop
                    }
                } else if(home == 's'){
                    // Settings option, currently just exits
                    std::cout << "Settings" << std::endl;
                    app = false;
                }
            } else {
                std::cout << "Login failed" << std::endl;
            }
        } else if(auth == 'r'){
            // Registration flow
            std::cout << "Username: ";
            std::string username;
            std::getline(std::cin, username);

            std::cout << "Password: ";
            std::string password;
            std::getline(std::cin, password);

            // Register new user via backend
            bool reg = Backend::Register(username, password);
            if(reg){
                std::map<int,std::string> array = Backend::GetUsers(username);

                system(CLEAR_COMMAND);
                std::cout << "Settings: s" << std::endl;

                // Display list of users
                for(auto& user : array){
                    std::cout << user.first << " : " << user.second << std::endl;
                }
                std::cout << "... - ";
                char home;
                std::cin >> home;
                std::cin.ignore();

                // If user chooses a number, start chat with that user
                if(isdigit(home)){
                    int index = home - '0';
                    if (array.find(index) != array.end()) {
                        std::string recipient = array[index];

                        running = true;
                        // Start chat updater and input handler threads
                        std::thread updater(ChatUpdater, username, recipient);
                        std::thread input(InputHandler, username, recipient);

                        input.join();  // Wait for input thread to finish (user typed /exit)
                        running = false;
                        updater.join(); // Then wait for updater thread to stop
                    }
                } else if(home == 's'){
                    // Settings option, currently just exits
                    std::cout << "Settings" << std::endl;
                    app = false;
                }
            }else{
                std::cout << "Error registering" << std::endl;
            }
            app = false;
        } else if(auth == 'c'){
            // Exit application
            app = false;
        }
    }
    return 0;
}
