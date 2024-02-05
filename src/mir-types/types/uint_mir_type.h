#ifndef UINT_MIR_TYPE_H
#define UINT_MIR_TYPE_H

#include "mir-types/mir_type.h"

class uint_mir_type : public mir_type {
public:
    uint_mir_type() : mir_type(
        std::regex (R"(^u(?:(?:8)|(?:16)|(?:32)|(?:64)|(?:size))$)"),
        [](const std::smatch &match) {
            return match[0].str();
        }
    ) {}
};

#endif //UINT_MIR_TYPE_H
