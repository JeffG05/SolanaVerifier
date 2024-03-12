#ifndef SOLANA_INSTRUCTION_MIR_VALUE_H
#define SOLANA_INSTRUCTION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class solana_instruction_mir_value : public mir_value {
public:
    solana_instruction_mir_value() : mir_value(
        std::regex (R"(^Instruction \{ program_id: (.+), accounts: (.+), data: (.+) \}$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1, add_ref1, remove_ref1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2, add_ref2, remove_ref2] = mir_value_converter::convert(match[2].str(), variables);
            auto [value3, returns3, add_ref3, remove_ref3] = mir_value_converter::convert(match[3].str(), variables);

            std::string value = "init_solana_instruction<" + value1 + ", " + value2 + ", " + value3 + ">";
            std::string add_refs = utils::add_item(utils::add_item(add_ref1, add_ref2, ", "), add_ref3, ", ");
            std::string remove_refs = utils::add_item(utils::add_item(remove_ref1, remove_ref2, ", "), remove_ref3, ", ");
            return std::make_tuple(value, true, add_refs, remove_refs);
        }
    ) {}
};

#endif //SOLANA_INSTRUCTION_MIR_VALUE_H
