#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "fibonacci.h"
#include "utils.h"
#include "eval.h"

// Main entry point for the Fibonacci project.
// Modes:
//   check_endianness : Check system endianness.
//   hex              : Use the first Fibonacci implementation.
//   hex2             : Use the alternate Fibonacci implementation.
//   eval             : Run evaluation mode.
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " {check_endianness|hex|hex2|eval} ..." << std::endl;
        return EXIT_FAILURE;
    }
    
    if (std::strcmp(argv[1], "check_endianness") == 0) {
        checkSystemEndianness();
    } else if (std::strcmp(argv[1], "hex") == 0) {
        if (argc < 3 || argc > 4) {
            std::cerr << "Usage: " << argv[0]
                      << " hex index [output.hex]" << std::endl;
            return EXIT_FAILURE;
        }
        char* endPtr = 0;
        uint64_t fibIndex = std::strtoull(argv[2], &endPtr, 10);
        if (*endPtr != '\0') {
            std::cerr << "Invalid index: " << argv[2] << std::endl;
            return EXIT_FAILURE;
        }
        Number resultNumber = fibonacci(fibIndex);
        std::cerr << "# Fibonacci index: " << fibIndex << std::endl;
        std::cerr << "# Result size: " << (resultNumber.digits.size() * sizeof(DIGIT)) << " B" 
                  << std::endl;
        if (argc == 4) {
            std::ofstream outputFile(argv[3]);
            if (!outputFile) {
                std::cerr << "Failed to open file: " << argv[3] << std::endl;
                return EXIT_FAILURE;
            }
            printNumberInHex(resultNumber, outputFile);
            outputFile.close();
        } else {
            printNumberInHex(resultNumber, std::cout);
        }
    } else if (std::strcmp(argv[1], "hex2") == 0) {
        if (argc < 3 || argc > 4) {
            std::cerr << "Usage: " << argv[0]
                      << " hex2 index [output.hex]" << std::endl;
            return EXIT_FAILURE;
        }
        char* endPtr = 0;
        uint64_t fibIndex = std::strtoull(argv[2], &endPtr, 10);
        if (*endPtr != '\0') {
            std::cerr << "Invalid index: " << argv[2] << std::endl;
            return EXIT_FAILURE;
        }
        Number resultNumber = fibonacci2(fibIndex);
        std::cerr << "# Fibonacci index (hex2): " << fibIndex << std::endl;
        std::cerr << "# Result size: " << (resultNumber.digits.size() * sizeof(DIGIT)) << " B" 
                  << std::endl;
        if (argc == 4) {
            std::ofstream outputFile(argv[3]);
            if (!outputFile) {
                std::cerr << "Failed to open file: " << argv[3] << std::endl;
                return EXIT_FAILURE;
            }
            printNumberInHex(resultNumber, outputFile);
            outputFile.close();
        } else {
            printNumberInHex(resultNumber, std::cout);
        }
    } else if (std::strcmp(argv[1], "eval") == 0) {
        runEvaluation();
    } else {
        std::cerr << "Unknown mode: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Program finished successfully." << std::endl;
    return EXIT_SUCCESS;
}
