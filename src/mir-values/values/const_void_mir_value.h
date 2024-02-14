#ifndef CONST_VOID_MIR_VALUE_H
#define CONST_VOID_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_void_mir_value : public mir_value {
public:
    const_void_mir_value() : mir_value(
        std::regex (R"(^const \(\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            return std::make_tuple("void", true, "", "");
        }
    ) {}
};

#endif //CONST_VOID_MIR_VALUE_H
