#ifndef MIR_TYPE_CONVERTER_H
#define MIR_TYPE_CONVERTER_H

#include<string>
#include "mir_type.h"

class mir_type_converter {
public:
    static std::string convert(const std::string& mir);
private:
    static std::vector<mir_type> _all_types;
};

#endif //MIR_TYPE_CONVERTER_H
