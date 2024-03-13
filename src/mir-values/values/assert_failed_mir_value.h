#ifndef ASSERT_FAILED_MIR_VALUE_H
#define ASSERT_FAILED_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class assert_failed_mir_value : public mir_value {
public:
    assert_failed_mir_value() : mir_value(
        std::regex (R"(^(?:core::panicking::)?assert_failed::<.+, .+>\(.+, .+, .+, .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            return std::make_tuple("return_error<>", false, "", "");
        }
    ) {}
};

#endif //ASSERT_FAILED_MIR_VALUE_H
