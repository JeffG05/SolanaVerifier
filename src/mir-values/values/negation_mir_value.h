#ifndef NEGATION_MIR_VALUE_H
#define NEGATION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class negation_mir_value : public mir_value {
public:
    negation_mir_value() : mir_value(
        std::regex (R"(^Neg\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);

            const std::optional<mir_statement> var = mir_statement::get_statement(variables, value.starts_with("state.") ? value.substr(6) : value);
            const std::string var_name = match[1].str();

            const std::regex num_const (R"(^const -?(?:\d+\.)?\d+(?:E[+-]\d+)?_?((?:[ui](?:8|16|32|64|size)|f(?:32|64)))$)");
            std::string var_type;
            if (var.has_value()) {
                var_type = var.value().get_ast_data().at("variable_type");
            } else if (std::smatch m1; std::regex_match(var_name, m1, num_const)) {
                var_type = m1[1].str();
            }

            for (char& c : var_type) {
                c = static_cast<char>(toupper(c));
            }

            if (match[1].str() == "const _") {
                value = "MAX_" + var_type;
            }

            std::string func = "negation(" + value + ", MAX_" + var_type;

            if (var_type.starts_with("U")) {
                std::string full_func = "unchecked<u_" + func + ")>";
                return std::make_tuple(full_func, true);
            }
            if (var_type.starts_with("I")) {
                std::string full_func = "unchecked<i_" + func + ", MIN_" + var_type + ")>";
                return std::make_tuple(full_func, true);
            }

            return std::make_tuple("UNSUPPORTED_" + func + ")", true);
        }
    ) {}
};

#endif //NEGATION_MIR_VALUE_H
