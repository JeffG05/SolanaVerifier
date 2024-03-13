include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

include(CMakeRC.cmake)
cmrc_add_resource_library(SolanaVerifierAssets assets/solana.casset)

set(Boost_NO_SYSTEM_PATHS TRUE)
set(Boost_NO_BOOST_CMAKE TRUE)
if (Boost_NO_SYSTEM_PATHS)
    set(BOOST_ROOT "/usr/local")
    set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
    set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.74 COMPONENTS program_options REQUIRED)
if (Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_INCLUDE_DIRS}")
endif ()
include_directories(${Boost_INCLUDE_DIRS})

add_executable(SolanaVerifier
        main.cpp
        solana_contract.cpp
        mir_contract.cpp
        solana_contract.h
        mir_contract.h
        mir_statement.cpp
        mir_statement.h
        utils.cpp
        utils.h
        c_program.cpp
        c_program.h
        controller.cpp
        controller.h
        verification_result.cpp
        verification_result.h
        mir-types/mir_type.cpp
        mir-types/mir_type.h
        mir-types/mir_type_converter.cpp
        mir-types/mir_type_converter.h
        mir-types/types/borrow_mir_type.h
        mir-types/types/uint_mir_type.h
        mir-types/types/int_mir_type.h
        mir-types/types/pubkey_mir_type.h
        mir-types/types/array_mir_type.h
        mir-types/types/account_info_mir_type.h
        mir-types/types/result_mir_type.h
        mir-types/types/void_mir_type.h
        mir-types/types/tuple_mir_type.h
        mir-values/mir_value.cpp
        mir-values/mir_value.h
        mir-values/mir_value_converter.cpp
        mir-values/mir_value_converter.h
        mir-values/values/ok_result_mir_value.h
        mir-values/values/const_number_mir_value.h
        mir-values/values/const_void_mir_value.h
        mir-values/values/multiplication_mir_value.h
        mir-values/values/addition_mir_value.h
        mir-values/values/variable_mir_value.h
        mir-values/values/move_mir_value.h
        mir-values/values/tuple_indexer_mir_value.h
        mir-values/values/tuple_indexer_mir_value.h
        mir-values/values/not_mir_value.h
        mir-values/values/function_mir_value.h
        mir-types/types/backtick_mir_type.h
        mir-types/types/string_mir_type.h
        mir-values/values/const_string_mir_value.h
        mir-values/values/print_mir_value.h
        mir-types/types/mutable_mir_type.h
        mir-types/types/ref_mir_type.h
        mir-types/types/ref_cell_mir_type.h
        mir-types/types/ref_mut_mir_type.h
        mir-types/types/rc_mir_type.h
        mir-types/types/infallible_mir_type.h
        mir-types/types/error_mir_type.h
        mir-types/types/control_flow_mir_type.h
        mir-types/types/iter_mir_type.h
        mir-values/values/iter_mir_value.h
        mir-values/values/borrow_mir_value.h
        mir-values/values/mutable_mir_value.h
        mir-values/values/not_equal_mir_value.h
        mir-values/values/deref_mir_value.h
        mir-values/values/program_error_mir_value.h
        mir-values/values/result_error_mir_value.h
        mir-values/values/as_continue_mir_value.h
        mir-values/values/as_break_mir_value.h
        mir-values/values/try_branch_result_mir_value.h
        mir-values/values/from_residual_mir_value.h
        mir-values/values/discriminant_mir_value.h
        mir-values/values/borrow_mutable_mir_value.h
        mir-values/values/next_mir_value.h
        mir-values/values/index_mut_range_full_mir_value.h
        hir_contract.cpp
        hir_contract.h
        mir-values/values/serialize_mir_value.h
        mir-values/values/deserialize_mir_value.h
        config.h
        mir-values/values/less_than_mir_value.h
        mir-values/values/greater_than_mir_value.h
        mir-values/values/less_than_equal_mir_value.h
        mir-values/values/greater_than_equal_mir_value.h
        mir-values/values/subtraction_mir_value.h
        mir-values/values/division_mir_value.h
        mir-values/values/number_operator_mir_value.h
        mir-values/values/equal_mir_value.h
        mir-values/values/negation_mir_value.h
        mir-values/values/const_number_max_mir_value.h
        mir-values/values/const_number_min_mir_value.h
        vulnerability.cpp
        vulnerability.h
        mir-values/values/bitand_mir_value.h
        mir-values/values/bitor_mir_value.h
        mir-values/values/bitxor_mir_value.h
        mir-types/types/vec_mir_type.h
        mir-types/types/account_meta_mir_type.h
        mir-types/types/solana_instruction_mir_type.h
        mir-types/types/deref_mir_type.h
        mir-types/types/box_mir_type.h
        mir-types/types/const_mir_type.h
        mir-values/values/solana_instruction_mir_value.h
        mir-values/values/init_array_mir_value.h
        mir-types/types/panic_assert_kind_mir_type.h
        mir-values/values/panic_assert_kind_mir_value.h
        mir-values/values/init_tuple_mir_value.h
        mir-values/values/promoted_mir_value.h
        mir-types/types/optional_mir_type.h
        mir-values/values/none_optional_mir_value.h
        mir-types/types/arguments_mir_type.h
        mir-values/values/assert_failed_mir_value.h
        mir-types/types/float_mir_type.h
        mir-values/values/conversion_mir_value.h
)

target_link_libraries(SolanaVerifier
        PRIVATE tomlplusplus::tomlplusplus
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE SolanaVerifierAssets
        PRIVATE ${Boost_LIBRARIES}
)

target_include_directories(SolanaVerifier
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)