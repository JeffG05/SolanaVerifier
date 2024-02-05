#ifndef RC_MIR_TYPE_H
#define RC_MIR_TYPE_H

#include "mir-types/mir_type.h"

class rc_mir_type : public mir_type {
public:
    rc_mir_type() : mir_type(
        std::regex(R"(^std::rc::Rc<(.+)>$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[1].str());
        }
    ) {}
};

#endif //RC_MIR_TYPE_H
