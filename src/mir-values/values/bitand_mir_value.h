#ifndef BITAND_MIR_VALUE_H
#define BITAND_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class bitand_mir_value : public mir_value {
public:
    bitand_mir_value() : mir_value(
        std::regex (R"(^BitAnd\((.+), (.+)\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            auto [value1, returns1, add_ref1, remove_ref1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2, add_ref2, remove_ref2] = mir_value_converter::convert(match[2].str(), variables);

            std::string value = "(" + value1 + " && " + value2 + ")";
            std::string add_ref = utils::add_item(add_ref1, add_ref2, ", ");
            std::string remove_ref = utils::add_item(remove_ref1, remove_ref2, ", ");
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //BITAND_MIR_VALUE_H
