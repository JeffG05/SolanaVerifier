#ifndef INTO_VEC_MIR_VALUE_H
#define INTO_VEC_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class into_vec_mir_value : public mir_value {
public:
    into_vec_mir_value() : mir_value(
        std::regex (R"(^std::slice::<impl \[.+?\]>::into_vec::<.+?>\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, returns);
        }
    ) {}
};

#endif //INTO_VEC_MIR_VALUE_H
