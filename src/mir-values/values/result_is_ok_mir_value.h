#ifndef RESULT_IS_OK_MIR_VALUE_H
#define RESULT_IS_OK_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class result_is_ok_mir_value : public mir_value {
public:
    result_is_ok_mir_value() : mir_value(
        std::regex (R"(^Result::<.*>::is_ok\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value + ".is_success", true);
        }
    ) {}
};

#endif //RESULT_IS_OK_MIR_VALUE_H
