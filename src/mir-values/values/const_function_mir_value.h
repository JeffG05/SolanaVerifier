#ifndef CONST_FUNCTION_MIR_VALUE_H
#define CONST_FUNCTION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_function_mir_value : public mir_value {
public:
    const_function_mir_value() : mir_value(
        std::regex (R"(^const (?:<.+>::)?(.+)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("const_func<" + std::regex_replace(match[1].str(), std::regex(R"(\W)"), "_") + "()>", true);
        }
    ) {}
};

#endif //CONST_FUNCTION_MIR_VALUE_H
