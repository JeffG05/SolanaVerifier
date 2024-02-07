#ifndef DEREF_MIR_TYPE_H
#define DEREF_MIR_TYPE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class deref_mir_value : public mir_value {
public:
    deref_mir_value() : mir_value(
        std::regex (R"(^(?:(?:\(\*(.+)\)|\*(.+))|<.+ as Deref>::deref\((.+)\))$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            std::string match_str;
            if (!match[1].str().empty()) {
                match_str = match[1].str();
            } else if (!match[2].str().empty()) {
                match_str = match[2].str();
            } else if (!match[3].str().empty()) {
                match_str = match[3].str();
            }

            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match_str, variables);
            std::string new_remove_ref = utils::add_item(remove_ref, value, ", ");
            return std::make_tuple(value, true, add_ref, new_remove_ref);
        }
    ) {}
};

#endif //DEREF_MIR_TYPE_H
