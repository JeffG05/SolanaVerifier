#ifndef NONE_OPTIONAL_MIR_VALUE_H
#define NONE_OPTIONAL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class none_optional_mir_value : public mir_value {
public:
    none_optional_mir_value() : mir_value(
        std::regex (R"(^Option::<.+>::None$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("optional_none<>", true, "", "");
        }
    ) {}
};

#endif //NONE_OPTIONAL_MIR_VALUE_H
