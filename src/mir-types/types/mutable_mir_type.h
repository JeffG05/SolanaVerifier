#ifndef MUTABLE_MIR_TYPE_H
#define MUTABLE_MIR_TYPE_H

#include "mir-types/mir_type.h"

class mutable_mir_type : public mir_type {
public:
    mutable_mir_type() : mir_type(
        std::regex(R"(^mut (.+)$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //MUTABLE_MIR_TYPE_H
