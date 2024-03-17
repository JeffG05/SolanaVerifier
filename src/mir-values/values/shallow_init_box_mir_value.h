#ifndef SHALLOW_INIT_BOX_MIR_VALUE_H
#define SHALLOW_INIT_BOX_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class shallow_init_box_mir_value : public mir_value {
public:
    shallow_init_box_mir_value() : mir_value(
        std::regex (R"(^ShallowInitBox\(.+?, .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //SHALLOW_INIT_BOX_MIR_VALUE_H
