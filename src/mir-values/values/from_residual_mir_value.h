#ifndef FROM_RESIDUAL_MIR_VALUE_H
#define FROM_RESIDUAL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class from_residual_mir_value : public mir_value {
public:
    from_residual_mir_value() : mir_value(
        std::regex (R"(^<.+ as FromResidual<.+>>::from_residual\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("from_residual<" + value + ">", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //FROM_RESIDUAL_MIR_VALUE_H
