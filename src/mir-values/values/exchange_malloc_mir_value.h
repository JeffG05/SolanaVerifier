#ifndef EXCHANGE_MALLOC_MIR_VALUE_H
#define EXCHANGE_MALLOC_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class exchange_malloc_mir_value : public mir_value {
public:
    exchange_malloc_mir_value() : mir_value(
        std::regex (R"(^alloc::alloc::exchange_malloc\(.+, .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("ignore<>", true);
        }
    ) {}
};

#endif //EXCHANGE_MALLOC_MIR_VALUE_H
