//
//  backend.hpp
//  Messenger
//
//  Created by АА on 28.05.25.
//

#ifndef backend_hpp
#define backend_hpp

#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

class Backend{
public:
    static bool Register(const std::string& username,const std::string& password);
    static bool Login(const std::string& username,const std::string& password);
    static bool SendMessage(const std::string& username,const std::string& friendname,const std::string& message);
    static std::vector<std::pair<std::string, std::string>> GetChat(const std::string& username,const std::string& friendname);
    static std::map<int,std::string> GetUsers(const std::string& users);
};


#endif /* backend_hpp */
