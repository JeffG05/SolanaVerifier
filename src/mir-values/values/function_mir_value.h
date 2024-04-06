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
            for (const auto& param : params) {
                auto [value, returns] = mir_value_converter::convert(param, variables);
                converted_params.push_back(value);
            }

            std::string function_name = match[1].str();
            auto function_name_parts = utils::split(function_name, ":");
            if (function_name_parts.size() > 1) {
                function_name = function_name_parts.front() + "__" + function_name_parts.back();
            }

            std::ranges::replace(function_name, ':', '_');

            return std::make_tuple(
                "func<" + utils::to_lower(function_name) + "(" + utils::join(converted_params, ", ") + ")>",
                true
            );
        }
    ) {}
};

#endif //FUNCTION_MIR_VALUE_H
