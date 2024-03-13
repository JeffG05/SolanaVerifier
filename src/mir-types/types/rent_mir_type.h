#ifndef RENT_MIR_TYPE_H
#define RENT_MIR_TYPE_H

#include "mir-types/mir_type.h"

class rent_mir_type : public mir_type {
public:
    rent_mir_type() : mir_type(
        std::regex (R"(^(?:solana_program::rent::)?Rent$)"),
        [](const std::smatch &match) {
            return "rent";
        }
    ) {}
};

#endif //RENT_MIR_TYPE_H
