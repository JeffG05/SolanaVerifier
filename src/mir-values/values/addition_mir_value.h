#ifndef ADDITION_MIR_VALUE_H
#define ADDITION_MIR_VALUE_H

#include "number_operator_mir_value.h"
#include "mir-types/mir_type_converter.h"

class addition_mir_value : public number_operator_mir_value {
public:
    addition_mir_value() : number_operator_mir_value("Add", "addition", false) {}
};

#endif //ADDITION_MIR_VALUE_H
