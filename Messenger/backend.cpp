//
//  backend.cpp
//  Messenger
//
//  Created by АА on 28.05.25.
//

#include "backend.hpp"
#include <iostream>
#include "json-2.hpp"
#include <string>
#include <curl/curl.h>
#include <thread>
#include <future>
#include <map>

using json = nlohmann::json;

/**
 * Callback function used by libcurl to write received data into a std::string.
 * Appends the data chunk received to the string pointed by userp.
 *
 * @param contents Pointer to the received data buffer.
 * @param size Size of each data element (usually 1).
 * @param nmemb Number of elements.
 * @param userp Pointer to std::string where data will be appended.
 * @return Number of bytes processed.
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * Registers a new user by sending username and password to the backend server.
 *
 * @param username New user's username.
 * @param password New user's password.
 * @return True if registration was successful, false otherwise.
 */
bool Backend::Register(const std::string& username, const std::string& password){
    CURL* curl = curl_easy_init();
    if(!curl) return false; // Failed to initialize curl

    std::string response;

    // Prepare JSON payload
    json j;
    j["username"] = username;
    j["password"] = password;
    std::string jsonStr = j.dump();

    // Set HTTP headers - content type JSON
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Set curl options for POST request to /register endpoint
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:4040/register");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request asynchronously to avoid blocking
    std::future<CURLcode> res = std::async(std::launch::async, [&]() {
        return curl_easy_perform(curl);
    });

    try {
        CURLcode result = res.get();
        if(result != CURLE_OK){
            // Cleanup curl resources and headers on failure
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return false;
        }

        // Parse JSON response from server
        json jsonResult = json::parse(response);

        // Return success flag from response
        bool success = jsonResult["success"];
        // Cleanup curl resources and headers on success
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return success;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        // Cleanup on exception
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return false;
    }
}

/**
 * Logs in a user by sending credentials to the backend server.
 *
 * @param username User's username.
 * @param password User's password.
 * @return True if login was successful, false otherwise.
 */
bool Backend::Login(const std::string& username, const std::string& password){
    CURL* curl = curl_easy_init();
    if(!curl) return false; // Failed to initialize curl

    std::string response;

    json j;
    j["username"] = username;
    j["password"] = password;
    std::string jsonStr = j.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:4040/login");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::future<CURLcode> res = std::async(std::launch::async, [&]() {
        return curl_easy_perform(curl);
    });

    try {
        CURLcode result = res.get();
        if(result != CURLE_OK){
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return false;
        }

        json jsonResult = json::parse(response);

        bool success = jsonResult["success"];
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return success;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return false;
    }
}

/**
 * Sends a chat message from username to friendname.
 *
 * @param username Sender's username.
 * @param friendname Recipient's username.
 * @param message Text message to send.
 * @return True if message was sent successfully, false otherwise.
 */
bool Backend::SendMessage(const std::string& username, const std::string& friendname, const std::string& message){
    CURL* curl = curl_easy_init();
    if(!curl) return false;

    std::string response;

    json j;
    j["username"] = username;
    j["friendname"] = friendname;
    j["message"] = message;

    std::string jsonStr = j.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:4040/send-message");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::future<CURLcode> res = std::async(std::launch::async, [&]() {
        return curl_easy_perform(curl);
    });

    try {
        CURLcode result = res.get();
        if(result != CURLE_OK){
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return false;
        }

        json jsonResult = json::parse(response);

        bool success = jsonResult["success"];
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return success;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return false;
    }
}

/**
 * Retrieves the chat history between username and friendname.
 *
 * @param username One chat participant.
 * @param friendname Other chat participant.
 * @return Vector of pairs, each containing sender's username and message.
 */
std::vector<std::pair<std::string, std::string>> Backend::GetChat(const std::string& username, const std::string& friendname){
    CURL* curl = curl_easy_init();
    if(!curl) return {}; // Failed to initialize curl

    std::string response;

    json j;
    j["username"] = username;
    j["friendname"] = friendname;

    std::string jsonStr = j.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:4040/get-chat");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::future<CURLcode> res = std::async(std::launch::async, [&]() {
        return curl_easy_perform(curl);
    });

    try {
        CURLcode result = res.get();
        if(result != CURLE_OK){
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return {};
        }

        json jsonResult = json::parse(response);

        std::vector<std::pair<std::string, std::string>> messages;

        // Extract sendername and message fields from each element in the JSON array
        for (auto& el : jsonResult) {
            if (el.contains("sendername") && el.contains("message")) {
                std::string sender = el["sendername"].get<std::string>();
                std::string message = el["message"].get<std::string>();
                messages.emplace_back(sender, message);
            } else {
                std::cerr << "Invalid chat message format\n";
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return messages;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return {};
    }
}

/**
 * Retrieves the list of users (except the requesting user).
 *
 * @param username Current user's username (to exclude from list).
 * @return Map of user IDs to usernames.
 */
std::map<int, std::string> Backend::GetUsers(const std::string& username){
    CURL* curl = curl_easy_init();
    if(!curl) return {}; // Failed to initialize curl

    std::string response;

    json j;
    j["username"] = username;

    std::string jsonStr = j.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:4040/get-users");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::future<CURLcode> res = std::async(std::launch::async, [&]() {
        return curl_easy_perform(curl);
    });

    try {
        CURLcode result = res.get();
        if(result != CURLE_OK){
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return {};
        }

        json jsonResult = json::parse(response);

        std::map<int, std::string> users;

        // Parse JSON array containing user info and fill the map
        for (auto& el : jsonResult) {
            int id = el["id"].get<int>();
            std::string username = el["username"].get<std::string>();
            users.insert({id, username});
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return users;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return {};
    }
}

