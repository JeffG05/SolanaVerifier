#ifndef RESULT_MIR_TYPE_H
#define RESULT_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class result_mir_type : public mir_type {
public:
    result_mir_type() : mir_type(
        std::regex (R"(^(?:std::result::)?Result<(.+), (.+)>$)"),
        [](const std::smatch &match) {
            return "result<" + mir_type_converter::convert(match[1].str()) + ">";
        }
    ) {}
};

#endif //RESULT_MIR_TYPE_H
