#ifndef MIR_VALUE_H
#define MIR_VALUE_H

#include <string>
#include <regex>
#include <functional>
#include <optional>

#include "mir_statement.h"

class mir_value {
public:
    explicit mir_value(const std::regex &pattern, const std::function<std::tuple<std::string, bool>(std::smatch, mir_statements)> &function);
    [[nodiscard]] std::optional<std::tuple<std::string, bool>> try_parse(const std::string& mir, const mir_statements& variables) const;

protected:
    std::regex _regex_pattern;
    std::function<std::tuple<std::string, bool>(std::smatch, mir_statements)> _parse_function;
};

#endif //MIR_VALUE_H
