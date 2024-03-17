#ifndef PUBKEY_AS_BYTES_MIR_VALUE_H
#define PUBKEY_AS_BYTES_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class pubkey_as_bytes_mir_value : public mir_value {
public:
    pubkey_as_bytes_mir_value() : mir_value(
        std::regex (R"(^Pubkey::to_bytes\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("pubkey_as_bytes<" + value + ">", true);
        }
    ) {}
};

#endif //PUBKEY_AS_BYTES_MIR_VALUE_H
