#ifndef INT_MIR_TYPE_H
#define INT_MIR_TYPE_H

#include "mir-types/mir_type.h"

class int_mir_type : public mir_type {
public:
    int_mir_type() : mir_type(
        std::regex (R"(^i(?:(?:8)|(?:16)|(?:32)|(?:64)|(?:size))$)"),
        [](const std::smatch &match) {
            return match[0].str();
        }
    ) {}
};

#endif //INT_MIR_TYPE_H
