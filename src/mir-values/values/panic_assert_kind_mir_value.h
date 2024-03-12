#ifndef PANIC_ASSERT_KIND_MIR_VALUE_H
#define PANIC_ASSERT_KIND_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class panic_assert_kind_mir_value : public mir_value {
public:
    panic_assert_kind_mir_value() : mir_value(
        std::regex (R"(^core::panicking::AssertKind::(.+)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            const std::string enum_value = utils::to_lower(match[1].str());
            return std::make_tuple("_panic_assert_kind_" + enum_value, true, "", "");
        }
    ) {}
};

#endif //PANIC_ASSERT_KIND_MIR_VALUE_H
