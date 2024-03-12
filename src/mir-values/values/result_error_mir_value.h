#ifndef RESULT_ERROR_MIR_VALUE_H
#define RESULT_ERROR_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class result_error_mir_value : public mir_value {
public:
    result_error_mir_value() : mir_value(
        std::regex (R"(^Result::<.*>::Err\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("result_error<" + value + ">", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //RESULT_ERROR_MIR_VALUE_H
