#ifndef CHECKED_SUB_MIR_VALUE_H
#define CHECKED_SUB_MIR_VALUE_H

#include <iostream>
#include <regex>

#include "mir-types/mir_type_converter.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class checked_sub_mir_value : public mir_value {
public:
    checked_sub_mir_value() : mir_value(
        std::regex (R"(^CheckedSub\((.+), (.+)\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {

            auto [a_value, a_returns, a_add, a_remove] = mir_value_converter::convert(match[1].str(), variables);
            auto [b_value, b_returns, b_add, b_remove] = mir_value_converter::convert(match[2].str(), variables);

            const std::optional<mir_statement> var1 = mir_statement::get_statement(variables, a_value.starts_with("state.") ? a_value.substr(6) : a_value);
            const std::optional<mir_statement> var2 = mir_statement::get_statement(variables, b_value.starts_with("state.") ? b_value.substr(6) : b_value);
            const std::string var1_name = match[1].str();
            const std::string var2_name = match[2].str();

            const std::regex num_const (R"(^const \d+_(.+)$)");
            std::string var_type;
            if (var1.has_value()) {
                var_type = var1.value().get_ast_data().at("variable_type");
            } else if (var2.has_value()) {
                var_type = var2.value().get_ast_data().at("variable_type");
            } else if (std::smatch m1; std::regex_match(var1_name, m1, num_const)) {
                var_type = m1[1].str();
            } else if (std::smatch m2; std::regex_match(var2_name, m2, num_const)) {
                var_type = m2[1].str();
            }

            for (char& c : var_type) {
                c = static_cast<char>(toupper(c));
            }

            if (match[1].str() == "const _") {
                a_value = "MAX_" + var_type;
            }
            if (match[2].str() == "const _") {
                b_value = "MIN_" + var_type;
            }

            std::string func = "subtraction(" + a_value + ", " + b_value + ", MAX_" + var_type;

            std::string add_refs;
            if (!a_add.empty() && !b_add.empty()) {
                add_refs = a_add + ", " + b_add;
            } else {
                add_refs = a_add + b_add;
            }

            std::string remove_refs;
            if (!a_remove.empty() && !b_remove.empty()) {
                remove_refs = a_remove + ", " + b_remove;
            } else {
                remove_refs = a_remove + b_remove;
            }

            if (var_type.starts_with("U")) {
                return std::make_tuple("u_" + func + ")", true, add_refs, remove_refs);
            }
            if (var_type.starts_with("I")) {
                return std::make_tuple("i_" + func + ", MIN_" + var_type + ")", true, add_refs, remove_refs);
            }

            return std::make_tuple("UNSUPPORTED_" + func + ")", true, add_refs, remove_refs);
            std::throw_with_nested(std::runtime_error("Unsupported addition: " + var_type));
        }
    ) {}
};

#endif //CHECKED_SUB_MIR_VALUE_H
