#ifndef NOT_MIR_VALUE_H
#define NOT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class not_mir_value : public mir_value {
public:
    not_mir_value() : mir_value(
        std::regex (R"(^!(.+)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("(!(" + value + "))", returns, add_ref, remove_ref);
        }
    ) {}
};

#endif //NOT_MIR_VALUE_H
