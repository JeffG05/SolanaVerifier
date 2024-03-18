#ifndef CREATE_ACCOUNT_MIR_VALUE_H
#define CREATE_ACCOUNT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class create_account_mir_value : public mir_value {
public:
    create_account_mir_value() : mir_value(
        std::regex (R"(^solana_program::system_instruction::create_account\((.+?), (.+?), (.+?), (.+?), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            auto [value3, returns3] = mir_value_converter::convert(match[3].str(), variables);
            auto [value4, returns4] = mir_value_converter::convert(match[4].str(), variables);
            if (value4 == "promoted<>") {
                value4 = "0";
            }
            auto [value5, returns5] = mir_value_converter::convert(match[5].str(), variables);
            return std::make_tuple("sol_create_account(" + value1 + ", " + value2 + ", " + value3 + ", " + value4 + ", " + value5 + ", SYSTEM_PROGRAM_ID)", true);
        }
    ) {}
};

#endif //CREATE_ACCOUNT_MIR_VALUE_H
