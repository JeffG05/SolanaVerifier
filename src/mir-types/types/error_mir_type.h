#ifndef ERROR_MIR_TYPE_H
#define ERROR_MIR_TYPE_H

#include "mir-types/mir_type.h"

class error_mir_type : public mir_type {
public:
    error_mir_type() : mir_type(
        std::regex (R"(^(?:solana_program::program_error::ProgramError|std::io::Error)$)"),
        [](const std::smatch &match) {
            return "string";
        }
    ) {}
};

#endif //ERROR_MIR_TYPE_H
