#ifndef CONST_NUMBER_MIR_VALUE_H
#define CONST_NUMBER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_number_mir_value : public mir_value {
public:
    const_number_mir_value() : mir_value(
        std::regex (R"(^const (-?(?:\d+\.)?\d+(?:E[+-]\d+)?)_?(?:[ui](?:8|16|32|64|size)|f(?:32|64))$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple(match[1].str(), true);
        }
    ) {}
};

#endif //CONST_NUMBER_MIR_VALUE_H
