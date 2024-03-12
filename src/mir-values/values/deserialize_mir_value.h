#ifndef DESERIALIZE_MIR_VALUE_H
#define DESERIALIZE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class deserialize_mir_value : public mir_value {
public:
    deserialize_mir_value() : mir_value(
        std::regex (R"(^<(.+) as BorshDeserialize>::(?:try_from_slice|deserialize)\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[2].str(), variables);
            const std::string deserialize_func = "deserialize_" + match[1].str();
            return std::make_tuple(deserialize_func + "(" + value + ")", true, add_ref, remove_ref);
        }
    ) {}
};

#endif //DESERIALIZE_MIR_VALUE_H
