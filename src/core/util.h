#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// Splits `input` on `delim`; empty fields are preserved except a trailing one
// (same behaviour as std::getline based splitting).
inline void split(const std::string &input, char delim, std::vector<std::string> &result) {
    std::string item;
    for (char c : input) {
        if (c == delim) {
            result.push_back(item);
            item.clear();
        } else {
            item.push_back(c);
        }
    }
    if (!item.empty())
        result.push_back(item);
}

inline std::string trim(const std::string &s) {
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto begin = std::find_if_not(s.begin(), s.end(), isSpace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    return begin < end ? std::string(begin, end) : std::string();
}

#endif
