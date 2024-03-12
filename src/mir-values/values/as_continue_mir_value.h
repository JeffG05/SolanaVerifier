#ifndef AS_CONTINUE_MIR_VALUE_H
#define AS_CONTINUE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class as_continue_mir_value : public mir_value {
public:
    as_continue_mir_value() : mir_value(
        std::regex (R"(^\((.+) as Continue\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("continue<" + value + ".continue_value>", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //AS_CONTINUE_MIR_VALUE_H
