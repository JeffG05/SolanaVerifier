#ifndef REF_MIR_TYPE_H
#define REF_MIR_TYPE_H

#include "mir-types/mir_type.h"

class ref_mir_type : public mir_type {
public:
    ref_mir_type() : mir_type(
        std::regex(R"(^std::cell::Ref<(.+), (.+)>$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert(match[2].str());
        }
    ) {}
};

#endif //REF_MIR_TYPE_H
