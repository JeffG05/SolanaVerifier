#ifndef PUBKEY_MIR_TYPE_H
#define PUBKEY_MIR_TYPE_H

#include "mir-types/mir_type.h"

class pubkey_mir_type : public mir_type {
public:
    pubkey_mir_type() : mir_type(
        std::regex (R"(^Pubkey$)"),
        [](const std::smatch &match) {
            return "pubkey";
        }
    ) {}
};

#endif //PUBKEY_MIR_TYPE_H
