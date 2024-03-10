#ifndef PANIC_ASSERT_KIND_MIR_TYPE_H
#define PANIC_ASSERT_KIND_MIR_TYPE_H

#include "mir-types/mir_type.h"

class panic_assert_kind_mir_type : public mir_type {
public:
    panic_assert_kind_mir_type() : mir_type(
        std::regex (R"(^core::panicking::AssertKind$)"),
        [](const std::smatch &match) {
            return "panic_assert_kind";
        }
    ) {}
};

#endif //PANIC_ASSERT_KIND_MIR_TYPE_H
