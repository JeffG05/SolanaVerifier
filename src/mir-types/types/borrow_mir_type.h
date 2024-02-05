#ifndef BORROW_MIR_TYPE_H
#define BORROW_MIR_TYPE_H

#include "mir-types/mir_type.h"

class borrow_mir_type : public mir_type {
public:
    borrow_mir_type() : mir_type(
        std::regex(R"(^&(.+)$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //BORROW_MIR_TYPE_H
