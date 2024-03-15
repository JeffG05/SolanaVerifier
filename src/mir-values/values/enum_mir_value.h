#ifndef ENUM_MIR_VALUE_H
#define ENUM_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class enum_mir_value : public mir_value {
public:
    enum_mir_value() : mir_value(
        std::regex (R"(^([^:\s]+)::([^:\s]+?)(?: \{ (.+) \})?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            std::string value = "init_enum<" + utils::to_lower(match[2].str());
            if (match[3].matched) {
                const auto args = utils::split(match[3].str(), ", ");
                for (const auto& arg: args) {
                    std::string arg_var = utils::split(arg, ": ").back();
                    auto [arg_value, arg_returns] = mir_value_converter::convert(arg_var, variables);
                    value += ", " + arg_value;
                }
            }
            value += ">";
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //ENUM_MIR_VALUE_H
