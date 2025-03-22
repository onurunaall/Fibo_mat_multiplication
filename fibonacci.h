#ifndef FIBONACCI_H
#define FIBONACCI_H

#include <cstdint>
#include <vector>

// Use appropriate types for DIGIT and DBDGT based on debug mode
#ifdef DEBUG
typedef uint32_t DIGIT;
typedef uint64_t DBDGT;
#else
typedef uint64_t DIGIT;
typedef unsigned __int128 DBDGT;
#endif

// Number of bits per DIGIT and default tuple length for first implementation.
const int DIGIT_BIT = 8 * sizeof(DIGIT);
const int DEFAULT_TUPLE_LEN = 3;

// Structure to hold an arbitrary precision number.
struct Number {
    std::vector<DIGIT> digits; // Stored in little-endian order.
};

// Computes the Fibonacci number at the given index using matrix exponentiation (3-tuple version).
// Returns the result as a Number.
Number fibonacci(uint64_t index);

// Computes the Fibonacci number at the given index using an alternate method (2-tuple version).
// Returns the result as a Number.
Number fibonacci2(uint64_t index);

#endif // FIBONACCI_H
