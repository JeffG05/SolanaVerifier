#ifndef MIR_TYPE_H
#define MIR_TYPE_H

#include <string>
#include <regex>
#include <functional>
#include <optional>

class mir_type {
public:
    explicit mir_type(const std::regex &pattern, const std::function<std::string(std::smatch)> &function);
    [[nodiscard]] std::optional<std::string> try_parse(const std::string& mir) const;

protected:
    std::regex _regex_pattern;
    std::function<std::string(std::smatch)> _parse_function;
};



#endif //MIR_TYPE_H
