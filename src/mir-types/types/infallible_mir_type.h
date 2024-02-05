#ifndef INFALLIBLE_MIR_TYPE_H
#define INFALLIBLE_MIR_TYPE_H

#include "mir-types/mir_type.h"

class infallible_mir_type : public mir_type {
public:
    infallible_mir_type() : mir_type(
        std::regex (R"(^std::convert::Infallible$)"),
        [](const std::smatch &match) {
            return "infallible";
        }
    ) {}
};

#endif //INFALLIBLE_MIR_TYPE_H
