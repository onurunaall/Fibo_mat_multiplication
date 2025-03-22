#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <iostream>
#include "fibonacci.h"

// Checks the system endianness and prints "big" or "little".
void checkSystemEndianness();

// Prints a Number in hexadecimal format to the given output stream.
void printNumberInHex(const Number &bigNumber, std::ostream &outputStream);

#endif // UTILS_H
