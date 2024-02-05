#ifndef ACCOUNT_INFO_MIR_TYPE_H
#define ACCOUNT_INFO_MIR_TYPE_H

#include "mir-types/mir_type.h"

class account_info_mir_type : public mir_type {
public:
    account_info_mir_type() : mir_type(
        std::regex (R"(^(?:solana_program::account_info::)?AccountInfo<.+>$)"),
        [](const std::smatch &match) {
            return "account_info";
        }
    ) {}
};

#endif //ACCOUNT_INFO_MIR_TYPE_H
