#ifndef ACCOUNT_META_MIR_TYPE_H
#define ACCOUNT_META_MIR_TYPE_H

#include "mir-types/mir_type.h"

class account_meta_mir_type : public mir_type {
public:
    account_meta_mir_type() : mir_type(
        std::regex (R"(^(?:solana_program::instruction::)?AccountMeta$)"),
        [](const std::smatch &match) {
            return "account_meta";
        }
    ) {}
};

#endif //ACCOUNT_META_MIR_TYPE_H
