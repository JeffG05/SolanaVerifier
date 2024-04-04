#ifndef SERIALIZE_MIR_VALUE_H
#define SERIALIZE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class serialize_mir_value : public mir_value {
public:
    serialize_mir_value() : mir_value(
        std::regex (R"(^<(.+) as BorshSerialize>::(?:try_to_vec|serialize::<.+>)\((.+?)(?:, (.+))?\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[2].str(), variables);
            auto type = mir_type_converter::convert(match[1].str());
            const std::string serialize_func = "serialize<" + type + ", ";

            if (match[3].matched) {
                auto [value2, returns2] = mir_value_converter::convert(match[3].str(), variables);
                return std::make_tuple(serialize_func + value1 + ", " + value2 + ">", true);
            }

            return std::make_tuple(serialize_func + value1 + ">", true);
        }
    ) {}
};

#endif //SERIALIZE_MIR_VALUE_H
