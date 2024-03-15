#ifndef NEXT_MIR_VALUE_H
#define NEXT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class next_mir_value : public mir_value {
public:
    next_mir_value() : mir_value(
        std::regex (R"(^next_account_info::<.+>\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("next<" + value + ">", true);
        }
    ) {}
};

#endif //NEXT_MIR_VALUE_H
