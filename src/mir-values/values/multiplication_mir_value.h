#ifndef MULTIPLICATION_MIR_VALUE_H
#define MULTIPLICATION_MIR_VALUE_H

#include "number_operator_mir_value.h"
#include "mir-values/mir_value_converter.h"

class multiplication_mir_value : public number_operator_mir_value {
public:
    multiplication_mir_value() : number_operator_mir_value("Mul", "multiplication", false) {}
};

#endif //MULTIPLICATION_MIR_VALUE_H
