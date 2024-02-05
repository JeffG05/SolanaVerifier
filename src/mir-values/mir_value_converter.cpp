#include "mir_value_converter.h"

#include "values/checked_add_mir_value.h"
#include "values/checked_mul_mir_value.h"
#include "values/const_number_mir_value.h"
#include "values/const_string_mir_value.h"
#include "values/const_void_mir_value.h"
#include "values/function_mir_value.h"
#include "values/move_mir_value.h"
#include "values/negation_mir_value.h"
#include "values/ok_result_mir_value.h"
#include "values/print_mir_value.h"
#include "values/tuple_indexer_mir_value.h"
#include "values/variable_mir_value.h"

std::tuple<std::string, bool, std::string, std::string> mir_value_converter::convert(const std::string& mir, const std::list<mir_statement>& variables) {
    for (const auto& value: _all_values) {
        if (auto parsed = value.try_parse(mir, variables); parsed.has_value()) {
            return parsed.value();
        }
    }
    return std::make_tuple(mir, false, "", "");
}

std::vector<mir_value> mir_value_converter::_all_values = {
    ok_result_mir_value(),
    const_number_mir_value(),
    const_string_mir_value(),
    const_void_mir_value(),
    checked_mul_mir_value(),
    checked_add_mir_value(),
    variable_mir_value(),
    move_mir_value(),
    tuple_indexer_mir_value(),
    negation_mir_value(),
    print_mir_value(),

    // KEEP LAST
    function_mir_value()
};