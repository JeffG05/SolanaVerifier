#ifndef PROMOTED_MIR_VALUE_H
#define PROMOTED_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class promoted_mir_value : public mir_value {
public:
    promoted_mir_value() : mir_value(
        std::regex (R"(^const _$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("promoted<>", true);
        }
    ) {}
};

#endif //PROMOTED_MIR_VALUE_H
