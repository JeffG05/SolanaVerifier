#ifndef PANIC_MIR_VALUE_H
#define PANIC_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class panic_mir_value : public mir_value {
public:
    panic_mir_value() : mir_value(
        std::regex (R"(^(?:begin_)?panic(?:_cold_explicit)?(?:::<.+>)?\(.*\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("return_error<>", false);
        }
    ) {}
};

#endif //PANIC_MIR_VALUE_H
