#ifndef VEC_MIR_TYPE_H
#define VEC_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class vec_mir_type : public mir_type {
public:
    vec_mir_type() : mir_type(
        std::regex (R"(^(?:std::vec::)?Vec<(.+?)>$)"),
        [](const std::smatch &match) {
            return "array<" + mir_type_converter::convert(match[1].str()) + ">";
        }
    ) {}
};

#endif //VEC_MIR_TYPE_H
