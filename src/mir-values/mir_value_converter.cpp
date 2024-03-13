#include "mir_value_converter.h"
#include "values/as_break_mir_value.h"
#include "values/as_continue_mir_value.h"
#include "values/borrow_mir_value.h"
#include "values/borrow_mutable_mir_value.h"
#include "values/addition_mir_value.h"
#include "values/assert_failed_mir_value.h"
#include "values/bitand_mir_value.h"
#include "values/bitor_mir_value.h"
#include "values/bitxor_mir_value.h"
#include "values/const_number_max_mir_value.h"
#include "values/const_number_min_mir_value.h"
#include "values/division_mir_value.h"
#include "values/multiplication_mir_value.h"
#include "values/subtraction_mir_value.h"
#include "values/const_number_mir_value.h"
#include "values/const_string_mir_value.h"
#include "values/const_void_mir_value.h"
#include "values/conversion_mir_value.h"
#include "values/deref_mir_value.h"
#include "values/deserialize_mir_value.h"
#include "values/discriminant_mir_value.h"
#include "values/equal_mir_value.h"
#include "values/from_residual_mir_value.h"
#include "values/function_mir_value.h"
#include "values/greater_than_equal_mir_value.h"
#include "values/greater_than_mir_value.h"
#include "values/index_mut_range_full_mir_value.h"
#include "values/init_array_mir_value.h"
#include "values/init_tuple_mir_value.h"
#include "values/iter_mir_value.h"
#include "values/less_than_equal_mir_value.h"
#include "values/less_than_mir_value.h"
#include "values/move_mir_value.h"
#include "values/mutable_mir_value.h"
#include "values/negation_mir_value.h"
#include "values/not_mir_value.h"
#include "values/next_mir_value.h"
#include "values/none_optional_mir_value.h"
#include "values/not_equal_mir_value.h"
#include "values/ok_result_mir_value.h"
#include "values/panic_assert_kind_mir_value.h"
#include "values/print_mir_value.h"
#include "values/program_error_mir_value.h"
#include "values/promoted_mir_value.h"
#include "values/result_error_mir_value.h"
#include "values/serialize_mir_value.h"
#include "values/solana_instruction_mir_value.h"
#include "values/try_branch_result_mir_value.h"
#include "values/tuple_indexer_mir_value.h"
#include "values/variable_mir_value.h"

std::tuple<std::string, bool, std::string, std::string> mir_value_converter::convert(const std::string& mir, const mir_statements& variables) {
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
    multiplication_mir_value(),
    addition_mir_value(),
    subtraction_mir_value(),
    division_mir_value(),
    variable_mir_value(),
    move_mir_value(),
    tuple_indexer_mir_value(),
    not_mir_value(),
    print_mir_value(),
    iter_mir_value(),
    borrow_mir_value(),
    mutable_mir_value(),
    not_equal_mir_value(),
    deref_mir_value(),
    program_error_mir_value(),
    result_error_mir_value(),
    as_continue_mir_value(),
    as_break_mir_value(),
    try_branch_mir_value(),
    from_residual_mir_value(),
    discriminant_mir_value(),
    borrow_mutable_mir_value(),
    next_mir_value(),
    index_mut_range_full_mir_value(),
    serialize_mir_value(),
    deserialize_mir_value(),
    less_than_mir_value(),
    greater_than_mir_value(),
    less_than_equal_mir_value(),
    greater_than_equal_mir_value(),
    equal_mir_value(),
    negation_mir_value(),
    const_number_max_mir_value(),
    const_number_min_mir_value(),
    bitand_mir_value(),
    bitor_mir_value(),
    bitxor_mir_value(),
    solana_instruction_mir_value(),
    init_array_mir_value(),
    panic_assert_kind_mir_value(),
    init_tuple_mir_value(),
    none_optional_mir_value(),
    assert_failed_mir_value(),
    conversion_mir_value(),

    // KEEP LAST
    promoted_mir_value(),
    function_mir_value()
};