#ifndef INIT_TUPLE_MIR_VALUE_H
#define INIT_TUPLE_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class init_tuple_mir_value : public mir_value {
public:
    init_tuple_mir_value() : mir_value(
        std::regex (R"(^\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            const std::list<std::string> contents = utils::split(match[1].str(), ", ");
            std::list<std::string> converted_contents;
            std::list<std::string> add_refs;
            std::list<std::string> remove_refs;

            for (const auto& content : contents) {
                auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(content, variables);
                converted_contents.push_back(value);
                if (!add_ref.empty()) {
                    add_refs.push_back(add_ref);
                }
                if (!remove_ref.empty()) {
                    remove_refs.push_back(remove_ref);
                }
            }

            return std::make_tuple(
                "init_tuple<" + utils::join(converted_contents, ", ") + ">",
                true,
                utils::join(add_refs, ", "),
                utils::join(remove_refs, ", ")
            );
        }
    ) {}
};

#endif //INIT_TUPLE_MIR_VALUE_H
