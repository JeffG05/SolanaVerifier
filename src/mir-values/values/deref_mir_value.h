#ifndef DEREF_MIR_VALUE_H
#define DEREF_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class deref_mir_value : public mir_value {
public:
    deref_mir_value() : mir_value(
        std::regex (R"(^(?:\(\*(.+)\)|\*(.+)|<.+ as Deref>::deref\((.+)\)|<.+ as DerefMut>::deref_mut\((.+)\)|deref_copy (\(.+\)))$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            std::string match_str;
            for (int i = 1; i <= 5; i++) {
                if (!match[i].str().empty()) {
                    match_str = match[i].str();
                    break;
                }
            }

            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match_str, variables);
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //DEREF_MIR_VALUE_H
