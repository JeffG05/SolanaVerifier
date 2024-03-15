#ifndef TUPLE_INDEXER_MIR_VALUE_H
#define TUPLE_INDEXER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class tuple_indexer_mir_value : public mir_value {
public:
    tuple_indexer_mir_value() : mir_value(
        std::regex (R"(^\((.+)\.(\d+): .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [var, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            std::optional<mir_statement> var_statement = mir_statement::get_statement(variables, match[1].str());
            std::string value;
            if (var.starts_with("continue<")) {
                value = var.substr(9, var.size() - 10);
            } else if (var.starts_with("break<")) {
                value = var.substr(6, var.size() - 7);
            } else if (var_statement.has_value() && var_statement.value().get_ast_data().at("variable_type").get<std::string>().starts_with("array<")) {
                value = var + "[" + match[2].str() + "]";
            } else {
                value = var + ".get" + match[2].str();
            }
            const std::string new_add_ref = utils::add_item(add_ref, value, ", ");
            return std::make_tuple(value, true, new_add_ref, remove_ref);
        }
    ) {}
};

#endif //TUPLE_INDEXER_MIR_VALUE_H
