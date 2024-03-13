#ifndef CONVERSION_MIR_VALUE_H
#define CONVERSION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class conversion_mir_value : public mir_value {
public:
    conversion_mir_value() : mir_value(
        std::regex (R"(^(.+) as (.+?)(?: \(.+\))?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            const std::string raw_type = match[2].str();

            std::string converted_value = "((" + match[2].str() + ") " + value + ")";
            return std::make_tuple(converted_value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //CONVERSION_MIR_VALUE_H
