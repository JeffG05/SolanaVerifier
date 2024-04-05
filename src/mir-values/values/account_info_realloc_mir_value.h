#ifndef ACCOUNT_INFO_REALLOC_MIR_VALUE_H
#define ACCOUNT_INFO_REALLOC_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class account_info_realloc_mir_value : public mir_value {
public:
    account_info_realloc_mir_value() : mir_value(
        std::regex (R"(^AccountInfo::<.+>::realloc\((.+?), (.+), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ok<void>", true);
        }
    ) {}
};

#endif //ACCOUNT_INFO_REALLOC_MIR_VALUE_H
