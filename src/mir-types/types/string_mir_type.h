#ifndef STRING_MIR_TYPE_H
#define STRING_MIR_TYPE_H

#include "mir-types/mir_type.h"

class string_mir_type : public mir_type {
public:
    string_mir_type() : mir_type(
        std::regex (R"(^str$)"),
        [](const std::smatch &match) {
            return "string";
        }
    ) {}
};

#endif //STRING_MIR_TYPE_H
