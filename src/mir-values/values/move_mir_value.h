#ifndef MOVE_MIR_VALUE_H
#define MOVE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class move_mir_value : public mir_value {
public:
    move_mir_value() : mir_value(
        std::regex (R"(^move (.+)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, returns);
        }
    ) {}
};

#endif //MOVE_MIR_VALUE_H
