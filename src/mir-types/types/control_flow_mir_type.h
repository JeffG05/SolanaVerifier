#ifndef CONTROL_FLOW_MIR_TYPE_H
#define CONTROL_FLOW_MIR_TYPE_H

#include "mir-types/mir_type.h"

class control_flow_mir_type : public mir_type {
public:
    control_flow_mir_type() : mir_type(
        std::regex(R"(^std::ops::ControlFlow<([^<>,]+<[^<>]+>|[^<>,]+)(?:, ([^<>,]+<[^<>]+>|[^<>,]+))?>$)"),
        [](const std::smatch &match) {
            if (match[2].str().empty()) {
                return "controlflow<" + mir_type_converter::convert(match[1].str()) + ", " + mir_type_converter::convert("()") + ">";
            }
            return "controlflow<" + mir_type_converter::convert(match[1].str()) + ", " + mir_type_converter::convert(match[2].str()) + ">";
        }
    ) {}
};

#endif //CONTROL_FLOW_MIR_TYPE_H
