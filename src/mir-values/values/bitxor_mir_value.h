#ifndef BITXOR_MIR_VALUE_H
#define BITXOR_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class bitxor_mir_value : public mir_value {
public:
    bitxor_mir_value() : mir_value(
        std::regex (R"(^BitXor\((.+), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);

            std::string value = "((" + value1 + " || " + value2 + ") && !(" + value1 + " && " + value2 + "))" ;
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //BITXOR_MIR_VALUE_H
