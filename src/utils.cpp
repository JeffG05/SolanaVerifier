#include <sstream>
#include "utils.h"

#include "mir_statement.h"

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

std::string utils::join(const std::list<std::string> &l, const std::string &delim) {
    std::stringstream res;
    unsigned int i = 0;
    for (const auto& item: l) {
        res << item;
        if (i < l.size() - 1) {
            res << delim;
        }
        i++;
    }
    return res.str();
}

std::string utils::add_item(const std::string &s, const std::string &item, const std::string &delim) {
    if (s.empty()) {
        return item;
    }

    return s + delim + item;
}

std::string utils::clean(const std::string &s) {
    std::string clean;
    for (const char c : s) {
        if (isalnum(c)) {
            clean += static_cast<char>(tolower(c));
        }
    }
    return clean;
}

std::string utils::to_lower(const std::string &s) {
    std::string new_s = s;
    for (auto& c: new_s) {
        c = static_cast<char>(tolower(c));
    }
    return new_s;
}
