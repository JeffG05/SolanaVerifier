#ifndef RESULT_ERROR_MIR_TYPE_H
#define RESULT_ERROR_MIR_TYPE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class result_error_mir_value : public mir_value {
public:
    result_error_mir_value() : mir_value(
        std::regex (R"(^Result::<.*>::Err\((.+)\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("result_error<" + value + ">", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //RESULT_ERROR_MIR_TYPE_H
