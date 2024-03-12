#ifndef ARGUMENTS_MIR_TYPE_H
#define ARGUMENTS_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class arguments_mir_type : public mir_type {
public:
    arguments_mir_type() : mir_type(
        std::regex (R"(^(?:std::fmt::)?Arguments<.+>$)"),
        [](const std::smatch &match) {
            return "arguments";
        }
    ) {}
};

#endif //ARGUMENTS_MIR_TYPE_H
