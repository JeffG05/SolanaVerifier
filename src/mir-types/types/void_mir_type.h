#ifndef VOID_MIR_TYPE_H
#define VOID_MIR_TYPE_H

#include "mir-types/mir_type.h"

class void_mir_type : public mir_type {
public:
    void_mir_type() : mir_type(
        std::regex (R"(^(?:\(\)|!)$)"),
        [](const std::smatch &match) {
            return "void";
        }
    ) {}
};

#endif //VOID_MIR_TYPE_H
