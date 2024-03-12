#ifndef DISCRIMINANT_MIR_VALUE_H
#define DISCRIMINANT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class discriminant_mir_value : public mir_value {
public:
    discriminant_mir_value() : mir_value(
        std::regex (R"(^discriminant\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value + ".type == _continue ? 0 : (" + value + ".type == _break ? 1 : 2)", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //DISCRIMINANT_MIR_VALUE_H
