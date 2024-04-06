#ifndef INIT_ENUM_MIR_VALUE_H
#define INIT_ENUM_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"
#include "mir-values/values/init_struct_mir_value.h"

class init_enum_mir_value : public mir_value {
public:
    init_enum_mir_value() : mir_value(
        std::regex (R"(^([^:\s]+)::([^:\s]+?)(?: \{ (.+) \})?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            std::optional<mir_statement> struct_statement = mir_statement::get_statement(variables, utils::to_lower(match[2].str()));

            if (struct_statement.has_value()) {
                std::string struct_type = struct_statement.value().get_ast_data().at("variable_type");
                if (struct_type != "Enum" && struct_type != "Enum Option") {
                    return init_struct_mir_value().try_parse(match[0].str(), variables).value();
                }
            }

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

#endif //INIT_ENUM_MIR_VALUE_H
