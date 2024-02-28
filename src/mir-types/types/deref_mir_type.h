#ifndef DEREF_MIR_TYPE_H
#define DEREF_MIR_TYPE_H

#include "mir-types/mir_type.h"

class deref_mir_type : public mir_type {
public:
    deref_mir_type() : mir_type(
        std::regex(R"(^\*(.+)$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //DEREF_MIR_TYPE_H
