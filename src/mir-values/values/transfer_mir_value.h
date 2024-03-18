#ifndef TRANSFER_MIR_VALUE_H
#define TRANSFER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class transfer_mir_value : public mir_value {
public:
    transfer_mir_value() : mir_value(
        std::regex (R"(^transfer\((.+?), (.+?), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            auto [value3, returns3] = mir_value_converter::convert(match[3].str(), variables);
            return std::make_tuple("sol_transfer(" + value1 + ", " + value2 + ", " + value3 + ", SYSTEM_PROGRAM_ID)", true);
        }
    ) {}
};

#endif //TRANSFER_MIR_VALUE_H
