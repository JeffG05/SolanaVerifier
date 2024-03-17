#ifndef ALIGN_OF_MIR_VALUE_H
#define ALIGN_OF_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class align_of_mir_value : public mir_value {
public:
    align_of_mir_value() : mir_value(
        std::regex (R"(^AlignOf\(.+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //ALIGN_OF_MIR_VALUE_H
