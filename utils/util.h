#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <sstream>

void split(const std::string &input, char delim, std::vector<std::string> &result){
    std::stringstream ss(input);
    std::string item;
    while(std::getline(ss, item, delim)) {
        result.push_back(item);
    }
}




#endif