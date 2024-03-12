#ifndef AS_BREAK_MIR_VALUE_H
#define AS_BREAK_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class as_break_mir_value : public mir_value {
public:
    as_break_mir_value() : mir_value(
        std::regex (R"(^\((.+) as Break\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("break<" + value + ".break_value>", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //AS_BREAK_MIR_VALUE_H
