#ifndef SERIALIZE_MIR_VALUE_H
#define SERIALIZE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class serialize_mir_value : public mir_value {
public:
    serialize_mir_value() : mir_value(
        std::regex (R"(^<(.+) as BorshSerialize>::(?:try_to_vec|serialize::<.+>)\((.+?)(?:, (.+))?\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1, add_ref1, remove_ref1] = mir_value_converter::convert(match[2].str(), variables);
            const std::string serialize_func = "serialize<" + match[1].str() + ", ";

            if (match[3].matched) {
                auto [value2, returns2, add_ref2, remove_ref2] = mir_value_converter::convert(match[3].str(), variables);
                return std::make_tuple(serialize_func + value1 + ", " + value2 + ">", true, utils::add_item(add_ref1, add_ref2, ", "), utils::add_item(remove_ref1, remove_ref2, ", "));
            }

            return std::make_tuple(serialize_func + value1 + ">", true, add_ref1, remove_ref1);
        }
    ) {}
};

#endif //SERIALIZE_MIR_VALUE_H
