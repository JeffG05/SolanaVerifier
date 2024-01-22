#include "controller.h"

int main(const int argc, char* argv[]) {
    controller c;
    const bool success = c.run(argc, argv);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}