#include "mir_value.h"

mir_value::mir_value(const std::regex &pattern, const std::function<std::tuple<std::string, bool, std::string, std::string>(std::smatch, mir_statements)> &function) {
    _regex_pattern = pattern;
    _parse_function = function;
}

std::optional<std::tuple<std::string, bool, std::string, std::string>> mir_value::try_parse(const std::string& mir, const mir_statements& variables) const {
    if (std::smatch match; std::regex_match(mir, match, _regex_pattern)) {
        return _parse_function(match, variables);
    }
    return std::nullopt;
}