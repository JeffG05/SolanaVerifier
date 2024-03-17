#ifndef SYSTEM_PROGRAM_ID_MIR_VALUE_H
#define SYSTEM_PROGRAM_ID_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class system_program_id_mir_value : public mir_value {
public:
    system_program_id_mir_value() : mir_value(
        std::regex (R"(^solana_program::system_program::id\(\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("SYSTEM_PROGRAM_ID", true);
        }
    ) {}
};

#endif //SYSTEM_PROGRAM_ID_MIR_VALUE_H
