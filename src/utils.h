#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <string>

class utils {
public:
    static std::string trim(std::string s);
    static std::list<std::string> split(const std::string& s, const std::string& delim);
    static std::string join(const std::list<std::string>& l, const std::string& delim);
    static std::string add_item(const std::string& s, const std::string& item, const std::string& delim);
};



#endif //UTILS_H
