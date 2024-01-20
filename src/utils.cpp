#include "utils.h"

std::string utils::trim(std::string s) {
    auto itr = s.begin();
    while (itr != s.end() && isspace(*itr)) {
        ++itr;
    }
    if (itr != s.begin()) {
        s.erase(s.begin(), itr);
    }

    itr = s.end();
    while (itr != s.begin() && isspace(*itr)) {
        --itr;
    }
    if (itr != s.end()) {
        s.erase(itr+1);
    }

    return s;
}

std::list<std::string> utils::split(const std::string& s, const std::string& delim) {
    // Adapted from: https://stackoverflow.com/a/46931770
    size_t pos_start = 0;
    size_t pos_end;
    const size_t delim_len = delim.length();
    std::string token;
    std::list<std::string> tokens;

    while ((pos_end = s.find(delim, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        tokens.push_back(token);
    }

    if (!(token = s.substr(pos_start)).empty()) {
        tokens.push_back(token);
    }
    return tokens;
}



