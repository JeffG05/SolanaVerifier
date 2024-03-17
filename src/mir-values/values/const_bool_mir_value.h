#ifndef CONST_BOOL_MIR_VALUE_H
#define CONST_BOOL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_bool_mir_value : public mir_value {
public:
    const_bool_mir_value() : mir_value(
        std::regex (R"(^const (false|true)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple(match[1].str(), true);
        }
    ) {}
};

#endif //CONST_BOOL_MIR_VALUE_H
