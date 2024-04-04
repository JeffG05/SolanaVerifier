#ifndef STRUCT_MIR_TYPE_H
#define STRUCT_MIR_TYPE_H

#include "mir-types/mir_type.h"

class struct_mir_type : public mir_type {
public:
    struct_mir_type() : mir_type(
        std::regex(R"(^(?:.+::)+(.+)$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //STRUCT_MIR_TYPE_H
