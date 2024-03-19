#ifndef DATA_IS_EMPTY_MIR_VALUE_H
#define DATA_IS_EMPTY_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class data_is_empty_mir_value : public mir_value {
public:
    data_is_empty_mir_value() : mir_value(
        std::regex (R"(^AccountInfo::<.+?>::data_is_empty\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value + ".data_is_empty", true);
        }
    ) {}
};

#endif //DATA_IS_EMPTY_MIR_VALUE_H
