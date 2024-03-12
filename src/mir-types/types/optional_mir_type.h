#ifndef OPTIONAL_MIR_TYPE_H
#define OPTIONAL_MIR_TYPE_H

#include "mir-types/mir_type.h"
#include "mir-types/mir_type_converter.h"

class optional_mir_type : public mir_type {
public:
    optional_mir_type() : mir_type(
        std::regex (R"(^(?:std::option::)?Option<(.+)>$)"),
        [](const std::smatch &match) {
            return "optional<" + mir_type_converter::convert(match[1].str()) + ">";
        }
    ) {}
};

#endif //OPTIONAL_MIR_TYPE_H
