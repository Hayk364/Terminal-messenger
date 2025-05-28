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


//CallBack for dates from server
size_t WriteCallback(void* contents,size_t size,size_t nmemb,void* userp){
    ((std::string*)userp)->append((char*)contents,size * nmemb);
    return size * nmemb;
}

bool Backend::Register(const std::string& username,const std::string& password){
    CURL* curl = curl_easy_init();
    if(!curl) return false; //If init is not be true return false
    
    std::string response;
    
    json j;
    j["username"] = username;
    j["password"] = password; //Use json for send dates to server
    
    std::string jsonStr = j.dump();
    
    //HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL,"http://127.0.0.1:4040/register");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    
    
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
        return success;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return false;
    }
    
}

bool Backend::Login(const std::string& username,const std::string& password){
    CURL* curl = curl_easy_init();
    if(!curl) return false; //If init is not be true return false
    
    std::string response;
    
    json j;
    j["username"] = username;
    j["password"] = password; //Use json for send dates to server
    
    std::string jsonStr = j.dump();
    
    //HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL,"http://127.0.0.1:4040/login");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    
    
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

bool Backend::SendMessage(const std::string& username,const std::string& friendname,const std::string& message){
    CURL* curl = curl_easy_init();
    if(!curl) return false; //If init is not be true return false
    
    std::string response;
    
    json j;
    j["username"] = username;
    j["friendname"] = friendname;
    j["message"] = message;
    
    std::string jsonStr = j.dump();
    
    //HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL,"http://127.0.0.1:4040/send-message");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    
    
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

std::vector<std::pair<std::string, std::string>> Backend::GetChat(const std::string& username,const std::string& friendname){
    CURL* curl = curl_easy_init();
    if(!curl) return {}; //If init is not be true return false
    
    std::string response;
    
    json j;
    j["username"] = username;
    j["friendname"] = friendname;
    
    std::string jsonStr = j.dump();
    
    //HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL,"http://127.0.0.1:4040/get-chat");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    
    
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
        for (auto& el : jsonResult) {
            if (el.contains("sendername") && el.contains("message")) {
                std::string sender = el["sendername"].get<std::string>();
                std::string message = el["message"].get<std::string>();
                messages.emplace_back(sender, message);  // добавляем в вектор
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


std::map<int,std::string> Backend::GetUsers(const std::string& username){
    CURL* curl = curl_easy_init();
    if(!curl) return {}; //If init is not be true return false
    
    std::string response;
    
    json j;
    j["username"] = username;
    
    std::string jsonStr = j.dump();
    
    //HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL,"http://127.0.0.1:4040/get-users");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,jsonStr.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    
    
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
        
        std::map<int,std::string> users;
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
