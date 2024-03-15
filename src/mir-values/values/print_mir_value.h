#ifndef PRINT_MIR_VALUE_H
#define PRINT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class print_mir_value : public mir_value {
public:
    print_mir_value() : mir_value(
        std::regex (R"(^solana_program::log::sol_log\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("print(" + value + ")", false);
        }
    ) {}
};

#endif //PRINT_MIR_VALUE_H
