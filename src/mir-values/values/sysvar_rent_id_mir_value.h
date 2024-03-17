#ifndef SYSVAR_RENT_ID_MIR_VALUE_H
#define SYSVAR_RENT_ID_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class sysvar_rent_id_mir_value : public mir_value {
public:
    sysvar_rent_id_mir_value() : mir_value(
        std::regex (R"(^solana_program::sysvar::rent::id\(\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("SYSVAR_RENT_ID", true);
        }
    ) {}
};

#endif //SYSVAR_RENT_ID_MIR_VALUE_H
