#ifndef CONST_STRING_MIR_VALUE_H
#define CONST_STRING_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_string_mir_value : public mir_value {
public:
    const_string_mir_value() : mir_value(
        std::regex (R"(^const (\".+\")$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple(match[1].str(), true);
        }
    ) {}
};

#endif //CONST_STRING_MIR_VALUE_H
