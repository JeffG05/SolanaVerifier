#ifndef MIR_VALUE_CONVERTER_H
#define MIR_VALUE_CONVERTER_H

#include<string>
#include "mir_statement.h"
#include "mir_value.h"

class mir_value_converter {
public:
    static std::tuple<std::string, bool> convert(const std::string& mir, const mir_statements& variables);
private:
    static std::vector<mir_value> _all_values;
};

#endif //MIR_VALUE_CONVERTER_H
