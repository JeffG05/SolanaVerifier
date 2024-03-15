#ifndef TRY_BRANCH_MIR_VALUE_H
#define TRY_BRANCH_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class try_branch_mir_value : public mir_value {
public:
    try_branch_mir_value() : mir_value(
        std::regex (R"(^<Result<.+> as Try>::branch\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);

            std::optional<mir_statement> try_value = mir_statement::get_statement(variables, value.substr(6) + ".value");
            if (try_value.has_value()) {
                if (try_value.value().get_ast_data().at("variable_type") == "void") {
                    return std::make_tuple("try_void_branch<" + value + ".value>", true);
                }
            }
            return std::make_tuple("try_branch<" + value + ".value>", true);
        }
    ) {}
};

#endif //TRY_BRANCH_MIR_VALUE_H
