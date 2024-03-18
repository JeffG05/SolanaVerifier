#ifndef CONVERSION_MIR_VALUE_H
#define CONVERSION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class conversion_mir_value : public mir_value {
public:
    conversion_mir_value() : mir_value(
        std::regex (R"(^(.+) as (.+?)(?: \(.+\))?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            std::string type_value = mir_type_converter::convert(match[2].str());

            if (value == "promoted<>") {
                std::smatch type_match;
                std::regex num_type_regex (R"(^[ui](?:8|16|32|64|size)|f(?:8|16|32|64)$)");
                if (std::regex_match(type_value, type_match, num_type_regex)) {
                    value = "0";
                }
            }

            return std::make_tuple("conversion<" + value + ", " + type_value + ">", true);
        }
    ) {}
};

#endif //CONVERSION_MIR_VALUE_H
