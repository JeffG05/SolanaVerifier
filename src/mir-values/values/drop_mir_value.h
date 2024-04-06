#ifndef DROP_MIR_VALUE_H
#define DROP_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class drop_mir_value : public mir_value {
public:
    drop_mir_value() : mir_value(
        std::regex (R"(^(?:<.+>::)?drop\(.+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //DROP_MIR_VALUE_H
