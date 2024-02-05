#ifndef ARRAY_MIR_TYPE_H
#define ARRAY_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class array_mir_type : public mir_type {
public:
    array_mir_type() : mir_type(
        std::regex (R"(^\[(.+)\]$)"),
        [](const std::smatch &match) {
            return "array<" + mir_type_converter::convert(match[1].str()) + ">";
        }
    ) {}
};

#endif //ARRAY_MIR_TYPE_H
