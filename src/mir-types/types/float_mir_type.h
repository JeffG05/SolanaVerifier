#ifndef FLOAT_MIR_TYPE_H
#define FLOAT_MIR_TYPE_H

#include "mir-types/mir_type.h"

class float_mir_type : public mir_type {
public:
    float_mir_type() : mir_type(
        std::regex (R"(^f(?:(?:8)|(?:16)|(?:32)|(?:64))$)"),
        [](const std::smatch &match) {
            return match[0].str();
        }
    ) {}
};

#endif //FLOAT_MIR_TYPE_H
