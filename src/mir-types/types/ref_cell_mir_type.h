#ifndef REF_CELL_MIR_TYPE_H
#define REF_CELL_MIR_TYPE_H

#include "mir-types/mir_type.h"

class ref_cell_mir_type : public mir_type {
public:
    ref_cell_mir_type() : mir_type(
        std::regex(R"(^std::cell::RefCell<(.+)>$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //REF_CELL_MIR_TYPE_H
