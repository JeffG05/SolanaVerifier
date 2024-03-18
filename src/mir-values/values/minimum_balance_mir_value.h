#ifndef MINIMUM_BALANCE_MIR_VALUE_H
#define MINIMUM_BALANCE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class minimum_balance_mir_value : public mir_value {
public:
    minimum_balance_mir_value() : mir_value(
        std::regex (R"(^Rent::minimum_balance\((.+?), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            return std::make_tuple("sol_minimum_balance(" + value1 + ", " + value2 + ")", false);
        }
    ) {}
};

#endif //MINIMUM_BALANCE_MIR_VALUE_H
