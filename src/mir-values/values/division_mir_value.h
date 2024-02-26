#ifndef DIVISION_MIR_VALUE_H
#define DIVISION_MIR_VALUE_H

#include "number_operator_mir_value.h"
#include "mir-values/mir_value_converter.h"

class division_mir_value : public number_operator_mir_value {
public:
    division_mir_value() : number_operator_mir_value("Div", "division", true) {}
};

#endif //DIVISION_MIR_VALUE_H
