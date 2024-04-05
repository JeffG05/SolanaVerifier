#ifndef TRY_BORROW_MUTABLE_DATA_MIR_VALUE_H
#define TRY_BORROW_MUTABLE_DATA_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class try_borrow_mutable_data_mir_value : public mir_value {
public:
    try_borrow_mutable_data_mir_value() : mir_value(
        std::regex (R"(^AccountInfo::<.+>::try_borrow_mut_data\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("ok<" + value + ".get2>", true);
        }
    ) {}
};

#endif //TRY_BORROW_MUTABLE_DATA_MIR_VALUE_H
