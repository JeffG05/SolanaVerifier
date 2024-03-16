#ifndef INIT_ACCOUNT_META_MIR_VALUE_H
#define INIT_ACCOUNT_META_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class init_account_meta_mir_value : public mir_value {
public:
    init_account_meta_mir_value() : mir_value(
        std::regex (R"(^AccountMeta::new\((.+), (.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            return std::make_tuple(
                "init_account_meta<" + value1 + ", " + value2 + ", true>",
                true
            );
        }
    ) {}
};

#endif //INIT_ACCOUNT_META_MIR_VALUE_H
