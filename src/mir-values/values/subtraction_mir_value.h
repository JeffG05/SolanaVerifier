#ifndef SUBTRACTION_MIR_VALUE_H
#define SUBTRACTION_MIR_VALUE_H

#include "number_operator_mir_value.h"
#include "mir-values/mir_value_converter.h"

class subtraction_mir_value : public number_operator_mir_value {
public:
    subtraction_mir_value() : number_operator_mir_value("Sub", "subtraction", true) {}
};

#endif //SUBTRACTION_MIR_VALUE_H
