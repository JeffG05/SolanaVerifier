#ifndef CHECKED_MUL_MIR_VALUE_H
#define CHECKED_MUL_MIR_VALUE_H

#include <regex>

#include "mir-types/mir_type_converter.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class checked_mul_mir_value : public mir_value {
public:
    checked_mul_mir_value() : mir_value(
        std::regex (R"(^CheckedMul\((.+), (.+)\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            const std::string var_name = match[1].str();
            std::string var_type;
            const std::optional<mir_statement> var1 = mir_statement::get_statement(variables, var_name);
            if (var1.has_value()) {
                var_type = var1.value().get_ast_data().at("variable_type");
            } else {
                const std::regex num_const (R"(^const \d+_(.+)$)");

                if (std::smatch m; std::regex_match(var_name, m, num_const)) {
                    var_type = m[1].str();
                }
            }

            for (char& c : var_type) {
                c = static_cast<char>(toupper(c));
            }

            auto [a_value, a_returns, a_add, a_remove] = mir_value_converter::convert(match[1].str(), variables);
            auto [b_value, b_returns, b_add, b_remove] = mir_value_converter::convert(match[2].str(), variables);
            std::string func = "multiplication(" + a_value + ", " + b_value + ", MAX_" + var_type;

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
            std::throw_with_nested(std::runtime_error("Unsupported multiplication: " + var_type));
        }
    ) {}
};

#endif //CHECKED_MUL_MIR_VALUE_H
