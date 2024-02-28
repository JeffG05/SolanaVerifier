#ifndef CONST_MIR_TYPE_H
#define CONST_MIR_TYPE_H

#include "mir-types/mir_type.h"

class const_mir_type : public mir_type {
public:
    const_mir_type() : mir_type(
        std::regex(R"(^const (.+)$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //CONST_MIR_TYPE_H
