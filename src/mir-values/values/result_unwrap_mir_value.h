#ifndef RESULT_UNWRAP_MIR_VALUE_H
#define RESULT_UNWRAP_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class result_unwrap_mir_value : public mir_value {
public:
    result_unwrap_mir_value() : mir_value(
        std::regex (R"(^Result::<.+>::unwrap\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);

            std::optional<mir_statement> unwrap_value = mir_statement::get_statement(variables, value.substr(6) + ".value");
            if (unwrap_value.has_value()) {
                if (unwrap_value.value().get_ast_data().at("variable_type") == "void") {
                    return std::make_tuple("result_void_unwrap<" + value + ">", true);
                }
            }
            return std::make_tuple("result_unwrap<" + value + ">", true);
        }
    ) {}
};

#endif //RESULT_UNWRAP_MIR_VALUE_H
