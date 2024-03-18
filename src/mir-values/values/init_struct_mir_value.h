#ifndef INIT_STRUCT_MIR_VALUE_H
#define INIT_STRUCT_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class init_struct_mir_value : public mir_value {
public:
    init_struct_mir_value() : mir_value(
        std::regex (R"(^.+? \{ (.+) \}$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            const std::list<std::string> contents = utils::split(match[1].str(), ", ");
            std::list<std::string> converted_contents;

            for (const auto& content : contents) {
                auto [value, returns] = mir_value_converter::convert(utils::split(content, ": ").back(), variables);
                converted_contents.push_back(value);
            }
            return std::make_tuple(
                "init_tuple<" + utils::join(converted_contents, ", ") + ">",
                true
            );
        }
    ) {}
};

#endif //INIT_STRUCT_MIR_VALUE_H
