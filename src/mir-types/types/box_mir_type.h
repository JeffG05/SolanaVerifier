#ifndef BOX_MIR_TYPE_H
#define BOX_MIR_TYPE_H

#include "mir-types/mir_type.h"

class box_mir_type : public mir_type {
public:
    box_mir_type() : mir_type(
        std::regex(R"(^(?:std::boxed::)?Box<(.+)>$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //BOX_MIR_TYPE_H
