#ifndef ITER_MIR_VALUE_H
#define ITER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class iter_mir_value : public mir_value {
public:
    iter_mir_value() : mir_value(
        std::regex (R"(^core::slice::.+::iter\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //ITER_MIR_VALUE_H
