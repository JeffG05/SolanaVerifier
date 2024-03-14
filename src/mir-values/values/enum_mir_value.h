#ifndef ENUM_MIR_VALUE_H
#define ENUM_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class enum_mir_value : public mir_value {
public:
    enum_mir_value() : mir_value(
        std::regex (R"(^([^:\s]+)::([^:\s]+?)(?: \{ (.+) \})?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            std::string value = "init_enum<" + utils::to_lower(match[2].str());
            std::string add_refs;
            std::string remove_refs;
            if (match[3].matched) {
                auto args = utils::split(match[3].str(), ", ");
                for (const auto& arg: args) {
                    std::string arg_var = utils::split(arg, ": ").back();
                    auto [arg_value, arg_returns, arg_add_ref, arg_remove_ref] = mir_value_converter::convert(arg_var, variables);
                    value += ", " + arg_value;
                    add_refs = utils::add_item(add_refs, arg_add_ref, ",");
                    remove_refs = utils::add_item(remove_refs, arg_remove_ref, ",");
                }
            }
            value += ">";
            return std::make_tuple(value, true, add_refs, remove_refs);
        }
    ) {}
};

#endif //ENUM_MIR_VALUE_H
