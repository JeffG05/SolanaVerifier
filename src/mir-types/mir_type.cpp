#include "mir_type.h"

mir_type::mir_type(const std::regex &pattern, const std::function<std::string(std::smatch)> &function) {
    _regex_pattern = pattern;
    _parse_function = function;
}

std::optional<std::string> mir_type::try_parse(const std::string& mir) const {
    if (std::smatch match; std::regex_match(mir, match, _regex_pattern)) {
        return _parse_function(match);
    }
    return std::nullopt;
}




