#ifndef SOLANA_INSTRUCTION_MIR_TYPE_H
#define SOLANA_INSTRUCTION_MIR_TYPE_H

#include "mir-types/mir_type.h"

class solana_instruction_mir_type : public mir_type {
public:
    solana_instruction_mir_type() : mir_type(
        std::regex (R"(^(?:solana_program::instruction::)?Instruction)"),
        [](const std::smatch &match) {
            return "solana_instruction";
        }
    ) {}
};

#endif //SOLANA_INSTRUCTION_MIR_TYPE_H
