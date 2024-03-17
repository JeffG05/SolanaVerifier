#ifndef STR_AS_BYTES_MIR_VALUE_H
#define STR_AS_BYTES_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class str_as_bytes_mir_value : public mir_value {
public:
    str_as_bytes_mir_value() : mir_value(
        std::regex (R"(^core::str::<impl str>::as_bytes\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("str_as_bytes<" + value + ">", true);
        }
    ) {}
};

#endif //STR_AS_BYTES_MIR_VALUE_H
