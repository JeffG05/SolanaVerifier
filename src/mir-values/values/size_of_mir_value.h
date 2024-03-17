#ifndef SIZE_OF_MIR_VALUE_H
#define SIZE_OF_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class size_of_mir_value : public mir_value {
public:
    size_of_mir_value() : mir_value(
        std::regex (R"(^SizeOf\(.+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //SIZE_OF_MIR_VALUE_H
