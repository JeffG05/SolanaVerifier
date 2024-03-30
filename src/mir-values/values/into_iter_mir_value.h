#ifndef INTO_ITER_MIR_VALUE_H
#define INTO_ITER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class into_iter_mir_value : public mir_value {
public:
    into_iter_mir_value() : mir_value(
        std::regex (R"(^<.+>::into_iter\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, returns);
        }
    ) {}
};

#endif //INTO_ITER_MIR_VALUE_H
