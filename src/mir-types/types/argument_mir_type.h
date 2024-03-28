#ifndef ARGUMENT_MIR_TYPE_H
#define ARGUMENT_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class argument_mir_type : public mir_type {
public:
    argument_mir_type() : mir_type(
        std::regex (R"(^(?:core::fmt::rt::)?Argument<.+>$)"),
        [](const std::smatch &match) {
            return "argument";
        }
    ) {}
};

#endif //ARGUMENT_MIR_TYPE_H
