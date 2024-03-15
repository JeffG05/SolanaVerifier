#ifndef PROGRAM_ERROR_MIR_VALUE_H
#define PROGRAM_ERROR_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class program_error_mir_value : public mir_value {
public:
    program_error_mir_value() : mir_value(
        std::regex (R"(^solana_program::program_error::ProgramError::(.+)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("\"" + match[1].str() + "\"", true);
        }
    ) {}
};

#endif //PROGRAM_ERROR_MIR_VALUE_H
