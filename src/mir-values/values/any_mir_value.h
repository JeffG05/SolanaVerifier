#ifndef ANY_MIR_VALUE_H
#define ANY_MIR_VALUE_H

#include "utils.h"
#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class any_mir_value : public mir_value {
public:
    any_mir_value() : mir_value(
        std::regex (R"(^<.+>::any::<(.+)>\((.+?), .+\)$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value1, returns1] = mir_value_converter::convert(match[1].str(), variables);
            auto [value2, returns2] = mir_value_converter::convert(match[2].str(), variables);
            return std::make_tuple("any<" + value2 + ", " + std::regex_replace(value1, std::regex(R"(\W)"), "_") + "()>", true);
        }
    ) {}
};

#endif //ANY_MIR_VALUE_H
