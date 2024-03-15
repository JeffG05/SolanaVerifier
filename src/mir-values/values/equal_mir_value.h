#ifndef EQUAL_MIR_VALUE_H
#define EQUAL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class equal_mir_value : public mir_value {
public:
    equal_mir_value() : mir_value(
        std::regex (R"(^(?:Eq|<.+ as PartialEq>::eq)\((.+), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);

            std::string equality_type;
            std::optional<mir_statement> equality_var = mir_statement::get_statement(variables, value1.starts_with("state.") ? value1.substr(6) : value1);
            if (equality_var.has_value()) {
                equality_type = equality_var.value().get_ast_data().at("variable_type");
            }

            std::string value;
            if (equality_type == "pubkey" || equality_type == "string") {
                value = "is_equal_" + equality_type + "(" + value1 + ", " + value2 + ")";
            } else {
                value = "(" + value1 + " == " + value2 + ")";
            }
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //EQUAL_MIR_VALUE_H
