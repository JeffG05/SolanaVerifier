#ifndef PROGRAM_ERROR_MIR_TYPE_H
#define PROGRAM_ERROR_MIR_TYPE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class program_error_mir_value : public mir_value {
public:
    program_error_mir_value() : mir_value(
        std::regex (R"(^solana_program::program_error::ProgramError::(.+)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            return std::make_tuple("\"" + match[1].str() + "\"", true, "", "");
        }
    ) {}
};

#endif //PROGRAM_ERROR_MIR_TYPE_H
