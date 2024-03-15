#ifndef BOX_INDEXER_MIR_VALUE_H
#define BOX_INDEXER_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class box_indexer_mir_value : public mir_value {
public:
    box_indexer_mir_value() : mir_value(
        std::regex (R"(^\(\(\((.+?)\.0: std::ptr::Unique<.+?>\).0: std::ptr::NonNull<.+?>\).0: .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns, add_ref, remove_ref] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true, add_ref, remove_ref);
        }
    ) {}
};

#endif //BOX_INDEXER_MIR_VALUE_H
