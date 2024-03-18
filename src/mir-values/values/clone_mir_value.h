#ifndef CLONE_MIR_VALUE_H
#define CLONE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class clone_mir_value : public mir_value {
public:
    clone_mir_value() : mir_value(
        std::regex (R"(^<.+ as Clone>::clone\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //CLONE_MIR_VALUE_H
