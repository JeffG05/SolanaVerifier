#ifndef DISCRIMINANT_MIR_VALUE_H
#define DISCRIMINANT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class discriminant_mir_value : public mir_value {
public:
    discriminant_mir_value() : mir_value(
        std::regex (R"(^discriminant\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            const std::optional<mir_statement> var_statement = mir_statement::get_statement(variables, match[1].str());
            if (var_statement.has_value()) {
                std::string var_type = var_statement.value().get_ast_data().at("variable_type");
                if (var_type.starts_with("controlflow<")) {
                    return std::make_tuple("(" + value + ".type == _continue ? 0 : (" + value + ".type == _break ? 1 : 2))", true);
                }
                if (var_type.starts_with("result<")) {
                    return std::make_tuple("(" + value + ".is_success ? 0 : 1)", true);
                }
                return std::make_tuple("enum_discriminant<" + value + ">", true);
            }
            return std::make_tuple(static_cast<std::string>("unsupported_discriminant<>"), true);
        }
    ) {}
};

#endif //DISCRIMINANT_MIR_VALUE_H
