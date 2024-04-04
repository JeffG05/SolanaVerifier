#ifndef RENT_GET_MIR_VALUE_H
#define RENT_GET_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class rent_get_mir_value : public mir_value {
public:
    rent_get_mir_value() : mir_value(
        std::regex (R"(^<Rent as Sysvar>::get\(\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //RENT_GET_MIR_VALUE_H
