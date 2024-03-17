#ifndef CONVERSION_MIR_VALUE_H
#define CONVERSION_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class conversion_mir_value : public mir_value {
public:
    conversion_mir_value() : mir_value(
        std::regex (R"(^(.+) as (.+?)(?: \(.+\))?$)"),
        [](const std::smatch &match, const mir_statements& variables) {
            auto [value, returns] = mir_value_converter::convert(match[1].str(), variables);
            return std::make_tuple("conversion<" + value + ", " + mir_type_converter::convert(match[2].str()) + ">", true);
        }
    ) {}
};

#endif //CONVERSION_MIR_VALUE_H
