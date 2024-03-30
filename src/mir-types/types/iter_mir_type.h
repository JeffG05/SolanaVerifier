#ifndef ITER_MIR_TYPE_H
#define ITER_MIR_TYPE_H

#include "mir-types/mir_type.h"

class iter_mir_type : public mir_type {
public:
    iter_mir_type() : mir_type(
        std::regex(R"(^std::slice::Iter(?:Mut)?<([^<>,]+<[^<>]+>|[^<>,]+), ([^<>,]+<[^<>]+>|[^<>,]+)>$)"),
        [](const std::smatch &match) {
            return mir_type_converter::convert("[" + match[2].str() + "]");
        }
    ) {}
};

#endif //ITER_MIR_TYPE_H
