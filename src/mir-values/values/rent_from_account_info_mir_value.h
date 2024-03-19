#ifndef RENT_FROM_ACCOUNT_INFO_MIR_VALUE_H
#define RENT_FROM_ACCOUNT_INFO_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class rent_from_account_info_mir_value : public mir_value {
public:
    rent_from_account_info_mir_value() : mir_value(
        std::regex (R"(^<Rent as Sysvar>::from_account_info\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("ok<sol_from_account_info(" + value + ")>", true);
        }
    ) {}
};

#endif //RENT_FROM_ACCOUNT_INFO_MIR_VALUE_H
