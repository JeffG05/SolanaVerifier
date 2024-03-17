#ifndef FIND_PROGRAM_ADDRESS_MIR_VALUE_H
#define FIND_PROGRAM_ADDRESS_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class find_program_address_mir_value : public mir_value {
public:
    find_program_address_mir_value() : mir_value(
        std::regex (R"(^Pubkey::find_program_address\((.+?), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            return std::make_tuple("find_program_address<" + value1 + ", " + value2 + ">", true);
        }
    ) {}
};

#endif //FIND_PROGRAM_ADDRESS_MIR_VALUE_H
