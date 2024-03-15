#ifndef AS_BOX_MIR_VALUE_H
#define AS_BOX_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class as_box_mir_value : public mir_value {
public:
    as_box_mir_value() : mir_value(
        std::regex (R"(^move (.+) as std::boxed::Box<.+?>(?: \(.+\))?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple(value, true);
        }
    ) {}
};

#endif //AS_BOX_MIR_VALUE_H
