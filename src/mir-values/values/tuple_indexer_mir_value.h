#ifndef TUPLE_INDEXER_MIR_TYPE_H
#define TUPLE_INDEXER_MIR_TYPE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class tuple_indexer_mir_value : public mir_value {
public:
    tuple_indexer_mir_value() : mir_value(
        std::regex (R"(^\((.+)\.(\d+): .+\)$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            auto [var, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            const std::string value = var + ".get" + match[2].str();

            if (!add_ref.empty()) {
                add_ref += ", ";
            }
            add_ref += value;
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //TUPLE_INDEXER_MIR_TYPE_H
