#ifndef LAMPORTS_MIR_VALUE_H
#define LAMPORTS_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class lamports_mir_value : public mir_value {
public:
    lamports_mir_value() : mir_value(
        std::regex (R"(^AccountInfo::<.+>::lamports\((.+)\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value + ".get1", true);
        }
    ) {}
};

#endif //LAMPORTS_MIR_VALUE_H
