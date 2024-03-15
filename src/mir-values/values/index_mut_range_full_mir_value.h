#ifndef INDEX_MUT_RANGE_FULL_MIR_VALUE_H
#define INDEX_MUT_RANGE_FULL_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class index_mut_range_full_mir_value : public mir_value {
public:
    index_mut_range_full_mir_value() : mir_value(
        std::regex (R"(^<.+ as IndexMut<RangeFull>>::index_mut\((.+), const RangeFull\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //INDEX_MUT_RANGE_FULL_MIR_VALUE_H
