#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <string>
#include <filesystem>

class utils {
public:
    static std::string trim(std::string s);
    static std::list<std::string> split(const std::string& s, const std::string& delim, const int& max = -1);
    static std::string join(const std::list<std::string>& l, const std::string& delim);
    static std::string add_item(const std::string& s, const std::string& item, const std::string& delim);
    static std::string clean(const std::string& s);
    static std::string to_lower(const std::string& s);
    static std::string to_upper(const std::string& s);

    static bool contains_file(const std::filesystem::path &path) {
        auto recursion = std::filesystem::recursive_directory_iterator(path);
        return std::ranges::any_of(recursion, [](const std::filesystem::directory_entry &f) { return f.is_regular_file(); });
    }

    template <typename T>
    static void extend(std::list<T>* original, std::list<T> items) {
        original->splice(original->end(), items);
    };
};



#endif //UTILS_H
