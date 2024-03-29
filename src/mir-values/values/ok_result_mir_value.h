#ifndef OK_RESULT_MIR_VALUE_H
#define OK_RESULT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class ok_result_mir_value : public mir_value {
public:
    ok_result_mir_value() : mir_value(
        std::regex (R"(^Result::<.*, .*>::Ok\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [ok_type, ok_returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("ok<" + ok_type + ">", ok_returns);
        }
    ) {}
};

#endif //OK_RESULT_MIR_VALUE_H
