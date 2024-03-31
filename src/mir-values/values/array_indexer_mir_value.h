#ifndef ARRAY_INDEXER_MIR_VALUE_H
#define ARRAY_INDEXER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class array_indexer_mir_value : public mir_value {
public:
    array_indexer_mir_value() : mir_value(
        std::regex (R"(^\((.+)\)\[(.+)\]$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            return std::make_tuple(value1 + "[" + value2 + "]", true);
        }
    ) {}
};

#endif //ARRAY_INDEXER_MIR_VALUE_H
