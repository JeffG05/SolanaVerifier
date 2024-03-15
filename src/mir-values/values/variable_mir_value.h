#ifndef VARIABLE_MIR_VALUE_H
#define VARIABLE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class variable_mir_value : public mir_value {
public:
    variable_mir_value() : mir_value(
        std::regex (R"(^_\d+$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("state." + match[0].str(), true);
        }
    ) {}
};

#endif //VARIABLE_MIR_VALUE_H
