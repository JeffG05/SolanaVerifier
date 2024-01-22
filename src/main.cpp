#include <iostream>
#include <filesystem>
#include <ctime>
#include "solana_contract.h"
#include "mir_contract.h"

std::string get_datetime() {
    time_t rawtime;
    time(&rawtime);
    const tm* timeinfo = localtime(&rawtime);

    char datetime[28];
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", timeinfo);

    return datetime;
}

std::filesystem::path get_temp_dir() {
    const std::string package_name = "com.jeffgugelmann.SmartContractVerification";
    const std::filesystem::path tmp_dir = std::filesystem::temp_directory_path() / package_name / get_datetime();
    remove_all(tmp_dir);
    if (create_directories(tmp_dir)) {
        return tmp_dir;
    }
    std::throw_with_nested(std::runtime_error("Unable to create temporary dir"));
}

int main(const int argc, char* argv[])
{
    // Extract input parameters
    if (argc != 3) {
        std::cerr << "Expected 2 argument, received " << argc - 1 << std::endl;
        return 1;
    }
    const std::string contract_dir = argv[1];
    const std::string target_function = argv[2];

    // Load solana smart contract
    std::cout << "Searching smart contract in: " << contract_dir << std::endl;
    const auto contract = solana_contract(contract_dir);
    std::cout << "Smart contract found: " << contract.get_name() << std::endl;

    // Create temporary directory to store all intermediate files for this run
    const std::filesystem::path temp_dir = get_temp_dir();
    std::cout << "Creating temporary dir: " << temp_dir.string() << std::endl;

    // Convert smart contract to MIR
    const mir_contract mir = contract.convert_to_mir(temp_dir);
    std::cout << "MIR generated: " << mir.get_path() << std::endl;

    // Convert MIR to C Program
    c_program solana_c = mir.convert_to_c(temp_dir, target_function);
    std::cout << "C program generated: " << solana_c.get_path() << std::endl;

    std::cout << "Verifying Program" << std::endl;
    solana_c.verify();

    return 0;
}
