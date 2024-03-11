#ifndef EQUAL_MIR_VALUE_H
#define EQUAL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class equal_mir_value : public mir_value {
public:
    equal_mir_value() : mir_value(
        std::regex (R"(^(?:Eq|<.+ as PartialEq>::eq)\((.+), (.+)\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            auto [value1, returns1, add_ref1, remove_ref1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2, add_ref2, remove_ref2] = mir_value_converter::convert(match[2].str(), variables);

            std::string equality_type;
            std::optional<mir_statement> equality_var = mir_statement::get_statement(variables, value1.starts_with("state.") ? value1.substr(6) : value1);
            if (equality_var.has_value()) {
                equality_type = equality_var.value().get_ast_data().at("variable_type");
            }

            std::string value;
            if (equality_type == "pubkey" || equality_type == "string") {
                value = "is_equal(" + value1 + ", " + value2 + ")";
            } else {
                value = "(" + value1 + " == " + value2 + ")";
            }
            std::string add_ref = utils::add_item(add_ref1, add_ref2, ", ");
            std::string remove_ref = utils::add_item(remove_ref1, remove_ref2, ", ");
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //EQUAL_MIR_VALUE_H
