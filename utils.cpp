#include "utils.h"
#include <iostream>
#include <iomanip>
#include <climits>
#include <fstream>

// Checks system endianness and prints the result.
void checkSystemEndianness() {
    uint16_t testValue = 0xAABB;
    uint8_t *bytePtr = reinterpret_cast<uint8_t*>(&testValue);
    if (bytePtr[0] == 0xAA) {
        std::cout << "big" << std::endl;
    } else if (bytePtr[0] == 0xBB) {
        std::cout << "little" << std::endl;
    } else {
        std::cerr << "Unknown endianness." << std::endl;
    }
}

// Prints the given Number in hexadecimal format (most significant byte first).
void printNumberInHex(const Number &bigNumber, std::ostream &outputStream) {
    const unsigned char* byteData = reinterpret_cast<const unsigned char*>(bigNumber.digits.data());
    size_t numBytes = bigNumber.digits.size() * sizeof(DIGIT);
    size_t i;
    for (i = numBytes; i > 0; --i) {
        outputStream << std::hex << std::setw(2) << std::setfill('0')
                     << static_cast<int>(byteData[i - 1]);
    }
    outputStream << std::dec << std::endl;
}
