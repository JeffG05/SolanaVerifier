#include <iostream>
#include <boost/program_options.hpp>
#include "controller.h"
#include "mir_contract.h"
#include "solana_contract.h"
#include "utils.h"

controller::controller() {
    _globals = config();
}

bool controller::is_mir(const std::filesystem::path &path) {
    const std::string extension = path.extension().string();
    return utils::to_lower(extension) == ".mir";
}

bool controller::is_hir(const std::filesystem::path &path) {
    const std::string extension = path.extension().string();
    return utils::to_lower(extension) == ".hir";
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
    const std::filesystem::path tmp_dir = std::filesystem::temp_directory_path() / package_name / get_datetime() / "c";
    remove_all(tmp_dir);
    if (create_directories(tmp_dir)) {
        return tmp_dir.parent_path();
    }
    std::throw_with_nested(std::runtime_error("Unable to create temporary dir"));
}

void controller::print_info(const std::string &line) const {
    if (_globals.NO_DEBUG) return;
    std::cout << "INFO: " << line << std::endl;
}

void controller::print_warning(const std::string &line) const {
    if (_globals.NO_DEBUG) return;
    std::cout << "WARNING: " << line << std::endl;
}

void controller::print_error(const std::string &line) const {
    if (_globals.NO_DEBUG) return;
    std::cerr << "ERROR: " << line << std::endl;
}

void controller::print_report(const verification_result &result) {
    std::cout << std::endl;
    std::cout << "RESULT:" << std::endl;

    if (result.get_is_sat()) {
        std::cout << "No vulnerabilities found" << std::endl;
        return;
    }

    if (result.get_vulnerabilities().empty()) {
        std::cout << "Unable to run verification - " << result.get_error().value_or("unknown error occurred") << std::endl;
        return;
    }

    if (result.get_vulnerabilities().size() == 1) {
        std::cout << "1 vulnerability found" << std::endl;
    } else {
        std::cout << result.get_vulnerabilities().size() << " vulnerabilities found" << std::endl;
    }

    int i = 1;
    for (const auto& v : result.get_vulnerabilities()) {
        auto [v_name, v_description] = v.get_details();
        std::cout << std::endl;
        std::cout << "Vulnerability " << i << ": " << v_name << std::endl;
        std::cout << "-- Description: " << v_description << std::endl;
        if (v.get_reason().has_value()) {
            std::cout << "-- Reason: " << v.get_reason().value() << std::endl;
        }
        if (v.get_solution().has_value()) {
            std::cout << "-- Solution: " << v.get_solution().value() << std::endl;
        }
        ++i;
    }
}

bool controller::run(const int argc, char *argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Print help message")
        ("contract", boost::program_options::value<std::string>(&_contract_dir), "Directory of Solana smart contract")
        ("mir", boost::program_options::value<std::string>(&_mir_file), "MIR representation of Solana smart contract")
        ("hir", boost::program_options::value<std::string>(&_hir_file), "HIR representation of Solana smart contract")
        ("esbmc", boost::program_options::value<std::string>(&_esbmc_path)->default_value("esbmc"), "Path of the esbmc executable")
        ("no-debug", boost::program_options::bool_switch(&_globals.NO_DEBUG), "Turn off all debug statements")
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
        print_info("running SolanaVerifier on solana contract");

        if (!is_directory(_contract_dir)) {
            print_error("contract path provided is not a directory");
            return false;
        }
        if (vm.contains("mir")) {
            print_warning("ignoring mir file provided");
        }
        if (vm.contains("hir")) {
            print_warning("ignoring hir file provided");
        }

        auto t_contract_start = std::chrono::high_resolution_clock::now();
        const auto contract = solana_contract(_contract_dir, _globals);
        auto t_contract_end = std::chrono::high_resolution_clock::now();
        print_info("solana contract loaded from '" + contract.get_path() + "' (took " + get_millis(t_contract_start, t_contract_end) + ")");

        print_info("starting conversion to hir");
        auto t_to_hir_start = std::chrono::high_resolution_clock::now();
        hir = contract.convert_to_hir(temp_dir);
        auto t_to_hir_end = std::chrono::high_resolution_clock::now();
        if (!hir.has_value()) {
            print_error("failed to convert contract to hir");
            return false;
        }
        print_info("conversion to hir completed (took " + get_millis(t_to_hir_start, t_to_hir_end) + ")");
        print_info("hir saved to '" + hir->get_path() + "'");

        print_info("starting conversion to mir");
        auto t_to_mir_start = std::chrono::high_resolution_clock::now();
        mir = contract.convert_to_mir(temp_dir, hir->get_structs());
        auto t_to_mir_end = std::chrono::high_resolution_clock::now();
        if (!mir.has_value()) {
            print_error("failed to convert contract to mir");
            return false;
        }
        print_info("conversion to mir completed (took " + get_millis(t_to_mir_start, t_to_mir_end) + ")");
        print_info("mir saved to '" + mir->get_path() + "'");
    } else if (vm.contains("mir") && vm.contains("hir")) {
        print_info("running SolanaVerifier on mir and hir files");

        if (!is_mir(_mir_file)) {
            print_error("mir path provided is not a .mir file");
            return false;
        }

        if (!is_hir(_hir_file)) {
            print_error("hir path provided is not a .hir file");
            return false;
        }

        auto t_hir_start = std::chrono::high_resolution_clock::now();
        hir = hir_contract(_hir_file);
        auto t_hir_end = std::chrono::high_resolution_clock::now();
        if (!hir.has_value()) {
            print_error("failed to load hir");
            return false;
        }
        print_info("hir loaded from '" + hir->get_path() + "' (took " + get_millis(t_hir_start, t_hir_end) + ")");

        auto t_mir_start = std::chrono::high_resolution_clock::now();
        std::string contract_name = std::filesystem::path(_mir_file).stem().string();
        mir = mir_contract(contract_name, _mir_file, hir->get_structs(), _globals);
        auto t_mir_end = std::chrono::high_resolution_clock::now();
        if (!mir.has_value()) {
            print_error("failed to load mir");
            return false;
        }
        print_info("mir loaded from '" + mir->get_path() + "' (took " + get_millis(t_mir_start, t_mir_end) + ")");
    } else {
        print_error("no contract or mir and hir files provided");
        return false;
    }

    print_info("starting conversion to c program");
    auto t_to_c_start = std::chrono::high_resolution_clock::now();
    c_program solana_c = mir.value().convert_to_c(temp_dir / "c", hir->get_target());
    auto t_to_c_end = std::chrono::high_resolution_clock::now();
    print_info("conversion to c program completed (took " + get_millis(t_to_c_start, t_to_c_end) + ")");
    print_info("c program saved in '" + solana_c.get_dir() + "'");

    print_info("starting smt verification using boolector");
    const auto t_boolector_start = std::chrono::high_resolution_clock::now();
    const verification_result boolector_result = solana_c.verify_boolector(temp_dir, _esbmc_path);
    const auto t_boolector_end = std::chrono::high_resolution_clock::now();
    print_info("completed smt verification (took " + get_millis(t_boolector_start, t_boolector_end) + ")");

    print_info("printing vulnerability report");
    print_report(boolector_result);

    return true;
}


