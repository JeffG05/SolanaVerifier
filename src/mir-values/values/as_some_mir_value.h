#ifndef AS_SOME_MIR_VALUE_H
#define AS_SOME_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class as_some_mir_value : public mir_value {
public:
    as_some_mir_value() : mir_value(
        std::regex (R"(^(.+) as Some$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("some<" + value + ".value>", true);
        }
    ) {}
};

#endif //AS_SOME_MIR_VALUE_H
