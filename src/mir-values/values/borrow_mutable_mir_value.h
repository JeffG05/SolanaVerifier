#ifndef BORROW_MUTABLE_MIR_VALUE_H
#define BORROW_MUTABLE_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class borrow_mutable_mir_value : public mir_value {
public:
    borrow_mutable_mir_value() : mir_value(
        std::regex (R"(^.+::borrow_mut\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //BORROW_MUTABLE_MIR_VALUE_H
