#ifndef BORROW_MIR_VALUE_H
#define BORROW_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class borrow_mir_value : public mir_value {
public:
    borrow_mir_value() : mir_value(
        std::regex (R"(^(?:&(.+)|.+::borrow\((.+)\))$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            std::string mir_value;
            for(int i = 1; i <= 2; i++) {
                if (!match[i].str().empty()) {
                    mir_value = match[i].str();
                    break;
                }
            }
            auto [value, returns] = mir_value_converter::convert(mir_value, variables);
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //BORROW_MIR_VALUE_H
