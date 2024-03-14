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
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("result_unwrap<" + value + ">", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //RESULT_UNWRAP_MIR_VALUE_H
