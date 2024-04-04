#ifndef LEN_MIR_VALUE_H
#define LEN_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class len_mir_value : public mir_value {
public:
    len_mir_value() : mir_value(
        std::regex (R"(^(?:Len|.+::len)\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("len<" + value + ">", true);
        }
    ) {}
};

#endif //LEN_MIR_VALUE_H
