#include <iostream>
#include <boost/program_options.hpp>
#include "controller.h"
#include "mir_contract.h"
#include "solana_contract.h"

controller::controller() {
    _globals = config();
}

bool controller::is_mir(const std::filesystem::path &path) {
    std::string extension = path.extension().string();
    for (auto& c: extension) {
        c = static_cast<char>(tolower(c));
    }
    return extension == ".mir";
}

bool controller::is_hir(const std::filesystem::path &path) {
    std::string extension = path.extension().string();
    for (auto& c: extension) {
        c = static_cast<char>(tolower(c));
    }
    return extension == ".hir";
}

bool controller::is_directory(const std::filesystem::path &path) {
    return !path.has_extension();
}

std::string controller::get_millis(const std::chrono::time_point<std::chrono::high_resolution_clock> start, const std::chrono::time_point<std::chrono::high_resolution_clock> end) {
    const std::chrono::duration<double, std::milli> duration = end - start;
    return std::to_string(duration.count()) + "ms";
}

std::string controller::get_datetime() {
    time_t rawtime;
    time(&rawtime);
    const tm* timeinfo = localtime(&rawtime);

    char datetime[28];
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", timeinfo);

    return datetime;
}


std::filesystem::path controller::get_temp_dir() {
    const std::string package_name = "com.jeffgugelmann.SmartContractVerification";
    const std::filesystem::path tmp_dir = std::filesystem::temp_directory_path() / package_name / get_datetime();
    remove_all(tmp_dir);
    if (create_directories(tmp_dir)) {
        return tmp_dir;
    }
    std::throw_with_nested(std::runtime_error("Unable to create temporary dir"));
}


bool controller::run(const int argc, char *argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Print help message")
        ("contract", boost::program_options::value<std::string>(&_contract_dir), "Directory of Solana smart contract")
        ("mir", boost::program_options::value<std::string>(&_mir_file), "MIR representation of Solana smart contract")
        ("hir", boost::program_options::value<std::string>(&_hir_file), "HIR representation of Solana smart contract")
        ("target", boost::program_options::value<std::string>(&_target_function)->default_value("entrypoint"), "The target function to verify")
        ("esbmc", boost::program_options::value<std::string>(&_esbmc_path)->default_value("esbmc"), "Path of the esbmc executable")
        ("array-size", boost::program_options::value<int>(&_globals.ARRAY_SIZE)->default_value(10), "Max size of arrays in SMT model")
    ;

    boost::program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.contains("help")) {
        std::cout << desc << std::endl;
        return false;
    }

    std::optional<mir_contract> mir;
    std::optional<hir_contract> hir;
    const std::filesystem::path temp_dir = get_temp_dir();

    if (vm.contains("contract")) {
        std::cout << "Running SolanaVerifier on Rust contract" << std::endl;
        std::cout << std::endl;

        if (!is_directory(_contract_dir)) {
            std::cerr << "error: not a directory" << std::endl;
            return false;
        }
        if (vm.contains("mir")) {
            std::cerr << "warning: ignoring MIR file" << std::endl;
        }
        if (vm.contains("hir")) {
            std::cerr << "warning: ignoring HIR file" << std::endl;
        }

        auto t_contract_start = std::chrono::high_resolution_clock::now();
        const auto contract = solana_contract(_contract_dir, _globals);
        auto t_contract_end = std::chrono::high_resolution_clock::now();
        std::cout << "Loaded solana contract: took " << get_millis(t_contract_start, t_contract_end) << std::endl;
        std::cout << "-- Path: " << contract.get_path() << std::endl;
        std::cout << std::endl;

        std::cout << "Converting to HIR" << std::endl;
        auto t_to_hir_start = std::chrono::high_resolution_clock::now();
        hir = contract.convert_to_hir(temp_dir);
        auto t_to_hir_end = std::chrono::high_resolution_clock::now();
        if (!hir.has_value()) {
            std::cerr << "error: failed to generate hir file" << std::endl;
            return false;
        }
        std::cout << "Converted to HIR: took " << get_millis(t_to_hir_start, t_to_hir_end) << std::endl;
        std::cout << "-- Path: " << hir->get_path() << std::endl;
        std::cout << std::endl;

        std::cout << "Converting to MIR" << std::endl;
        auto t_to_mir_start = std::chrono::high_resolution_clock::now();
        mir = contract.convert_to_mir(temp_dir, hir->extract_structs());
        auto t_to_mir_end = std::chrono::high_resolution_clock::now();
        std::cout << "Converted to MIR: took " << get_millis(t_to_mir_start, t_to_mir_end) << std::endl;
        std::cout << "-- Path: " << mir->get_path() << std::endl;
        std::cout << std::endl;
    } else if (vm.contains("mir") && vm.contains("hir")) {
        std::cout << "Running SolanaVerifier on MIR file" << std::endl;
        std::cout << std::endl;

        if (!is_mir(_mir_file)) {
            if (is_directory(_mir_file)) {
                std::cerr << "error: expected .mir file not directory" << std::endl;
            } else {
                std::cerr << "error: incorrect file type ('" << std::filesystem::path(_mir_file).extension().string() << "')" << std::endl;
            }
            return false;
        }

        auto t_hir_start = std::chrono::high_resolution_clock::now();
        hir = hir_contract(_hir_file);
        auto t_hir_end = std::chrono::high_resolution_clock::now();
        if (!hir.has_value()) {
            std::cerr << "error: failed to generate hir file" << std::endl;
            return false;
        }
        std::cout << "Loaded HIR: took " << get_millis(t_hir_start, t_hir_end) << std::endl;
        std::cout << "-- Path: " << hir->get_path() << std::endl;
        std::cout << std::endl;

        auto t_mir_start = std::chrono::high_resolution_clock::now();
        std::string contract_name = std::filesystem::path(_mir_file).stem().string();
        mir = mir_contract(contract_name, _mir_file, hir->extract_structs(), _globals);
        auto t_mir_end = std::chrono::high_resolution_clock::now();
        std::cout << "Loaded MIR: took " << get_millis(t_mir_start, t_mir_end) << std::endl;
        std::cout << "-- Path: " << mir->get_path() << std::endl;
        std::cout << std::endl;
    } else {
        std::cerr << "error: expected contract or mir & hir file" << std::endl;
        return false;
    }

    if (!mir.has_value()) {
        std::cerr << "error: failed to generate mir file" << std::endl;
        return false;
    }

    auto t_to_c_start = std::chrono::high_resolution_clock::now();
    c_program solana_c = mir.value().convert_to_c(temp_dir, _target_function);
    auto t_to_c_end = std::chrono::high_resolution_clock::now();
    std::cout << "Converted to C: took " << get_millis(t_to_c_start, t_to_c_end) << std::endl;
    std::cout << "-- Path: " << solana_c.get_path() << std::endl;
    std::cout << std::endl;

    std::cout << "Running Verification using Boolector" << std::endl;
    const auto t_boolector_start = std::chrono::high_resolution_clock::now();
    const verification_result boolector_result = solana_c.verify_boolector(temp_dir, _esbmc_path);
    const auto t_boolector_end = std::chrono::high_resolution_clock::now();
    std::cout << "Completed Verification: took " << get_millis(t_boolector_start, t_boolector_end) << std::endl;
    std::cout << std::endl;

    if (boolector_result.get_is_sat()) {
        std::cout << "Result: No Vulnerability Found" << std::endl;
    } else {
        std::cout << "Result: Vulnerability Found" << std::endl;
        std::cout << "-- Vulnerability: " << boolector_result.get_vulnerability() << std::endl;
    }

    return true;
}


