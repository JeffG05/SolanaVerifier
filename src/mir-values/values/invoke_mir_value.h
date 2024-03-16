#ifndef INVOKE_MIR_VALUE_H
#define INVOKE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class invoke_mir_value : public mir_value {
public:
    invoke_mir_value() : mir_value(
        std::regex (R"(^invoke(?:_signed)?\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("invoke<>", false);
        }
    ) {}
};

#endif //INVOKE_MIR_VALUE_H
