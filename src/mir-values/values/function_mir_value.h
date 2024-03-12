#ifndef FUNCTION_MIR_VALUE_H
#define FUNCTION_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class function_mir_value : public mir_value {
public:
    function_mir_value() : mir_value(
        std::regex (R"(^(.+)\((.*)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            const std::list<std::string> params = utils::split(match[2].str(), ", ");
            std::list<std::string> converted_params;
            std::list<std::string> add_refs;
            std::list<std::string> remove_refs;
            for (const auto& param : params) {
                auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(param, variables);
                converted_params.push_back(value);
                if (!add_ref.empty()) {
                    add_refs.push_back(add_ref);
                }
                if (!remove_ref.empty()) {
                    remove_refs.push_back(remove_ref);
                }
            }

            std::string function_name = match[1].str();
            std::ranges::replace(function_name, ':', '_');

            return std::make_tuple(
                function_name + "(" + utils::join(converted_params, ", ") + ")",
                true,
                utils::join(add_refs, ", "),
                utils::join(remove_refs, ", ")
            );
        }
    ) {}
};

#endif //FUNCTION_MIR_VALUE_H
