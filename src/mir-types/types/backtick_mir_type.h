#ifndef BACKTICK_MIR_TYPE_H
#define BACKTICK_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class backtick_mir_type : public mir_type {
public:
    backtick_mir_type() : mir_type(
        std::regex (R"(^`(.+)`$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //BACKTICK_MIR_TYPE_H
