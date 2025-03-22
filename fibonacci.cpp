#include "fibonacci.h"
#include <cstdlib>
#include <cstring>
#include <climits>
#include <algorithm>
#include <future>
#include <chrono>
#include <iostream>

// Returns a pointer to the first part (A) of the 3-tuple matrix.
// This is a simple inline function so that it may be evaluated at compile time if possible.
static inline DIGIT* getMatrixA(DIGIT* basePtr, size_t numDigits) {
    return basePtr;
}

// Returns a pointer to the second part (B) of the 3-tuple matrix.
static inline DIGIT* getMatrixB(DIGIT* basePtr, size_t numDigits) {
    return basePtr + numDigits;
}

// Returns a pointer to the third part (C) of the 3-tuple matrix.
static inline DIGIT* getMatrixC(DIGIT* basePtr, size_t numDigits) {
    return basePtr + 2 * numDigits;
}

// Estimates how many DIGITs are needed for the Fibonacci number at the given index.
// Marked constexpr so that if fibIndex is known at compile time the result can be computed then.
static constexpr size_t estimateNumDigits(uint64_t fibIndex) {
    return (2 * fibIndex + DIGIT_BIT - 1) / DIGIT_BIT + 2;
}

// Multiplies sourceDigits by multiplier and accumulates the result in accum1 and accum2.
// One-line comment: Performs a scale-and-accumulate on an array of DIGITs.
static void scaleAndAccumulateOnce(DIGIT *accum1, DIGIT *accum2,
                                   const DIGIT *sourceDigits, DBDGT multiplier, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    for (i = 0; i < numDigits; ++i) {
        DBDGT product = ((DBDGT)sourceDigits[i]) * multiplier;
        DBDGT sum1 = ((DBDGT)accum1[i]) + product + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT;
        DBDGT sum2 = ((DBDGT)accum2[i]) + product + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT;
    }
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// Multiplies sourceDigits by two multipliers and accumulates results in accum1 and accum2.
// One-line comment: Simultaneously scales and accumulates with two different multipliers.
static void scaleAndAccumulateTwice(DIGIT *accum1, DIGIT *accum2,
                                    const DIGIT *sourceDigits, DBDGT multiplier1, DBDGT multiplier2, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    for (i = 0; i < numDigits; ++i) {
        DBDGT sourceVal = sourceDigits[i];
        DBDGT sum1 = ((DBDGT)accum1[i]) + sourceVal * multiplier1 + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT;
        DBDGT sum2 = ((DBDGT)accum2[i]) + sourceVal * multiplier2 + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT;
    }
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// Multiplies each digit of matrixB with matrixA and accumulates into accum arrays.
// One-line comment: Computes a multiplication pass using scaleAndAccumulateOnce.
static void multiplyOnce(DIGIT *accum1, DIGIT *accum2,
                           const DIGIT *matrixA, const DIGIT *matrixB,
                           size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAndAccumulateOnce(&accum1[i], &accum2[i],
                                 matrixA, matrixB[i], numDigitsA);
    }
}

// Multiplies matrixA by two multipliers from another matrix and accumulates into accum arrays.
// One-line comment: Computes a dual multiplication pass and returns the resulting digit count.
static size_t multiplyTwice(DIGIT *accum1, DIGIT *accum2,
                            const DIGIT *matrixA, const DIGIT *multiplierA, const DIGIT *multiplierB,
                            size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAndAccumulateTwice(&accum1[i], &accum2[i],
                                matrixA, multiplierA[i], multiplierB[i], numDigitsA);
    }
    size_t resultLength;
    for (resultLength = numDigitsA + numDigitsB; ; --resultLength) {
        if (accum1[resultLength] || accum2[resultLength]) {
            return resultLength + 1;
        }
    }
}

// Swaps two pointers to DIGIT arrays.
// One-line comment: Swaps two matrix pointers.
static void swapPointers(DIGIT *&ptr1, DIGIT *&ptr2) {
    DIGIT* temp = ptr1;
    ptr1 = ptr2;
    ptr2 = temp;
}

// Computes the Fibonacci number at the given index using matrix exponentiation.
// One-line comment: Main Fibonacci function (3-tuple version).
Number fibonacci(uint64_t fibIndex) {
    size_t estimatedDigits = estimateNumDigits(fibIndex);
    size_t totalBlockSize = 3 * DEFAULT_TUPLE_LEN * estimatedDigits;
    std::vector<DIGIT> matrixBlock(totalBlockSize, 0);
    
    // Set up pointers for our matrices.
    DIGIT* fibMatrix = matrixBlock.data();
    DIGIT* multiplierMatrix = fibMatrix + DEFAULT_TUPLE_LEN * estimatedDigits;
    DIGIT* workBuffer = fibMatrix + 2 * DEFAULT_TUPLE_LEN * estimatedDigits;
    
    size_t currentFibLength = 1;
    size_t currentMultiplierLength = 1;
    
    // Initialize fibMatrix to identity: A=1, B=0, C=1.
    getMatrixA(fibMatrix, estimatedDigits)[0] = 1;
    getMatrixB(fibMatrix, estimatedDigits)[0] = 0;
    getMatrixC(fibMatrix, estimatedDigits)[0] = 1;
    
    // Initialize multiplierMatrix to Fibonacci matrix: A=0, B=1, C=1.
    getMatrixA(multiplierMatrix, estimatedDigits)[0] = 0;
    getMatrixB(multiplierMatrix, estimatedDigits)[0] = 1;
    getMatrixC(multiplierMatrix, estimatedDigits)[0] = 1;
    
    // Process each bit of the exponent.
    while (fibIndex) {
        if (fibIndex & 1) {
            std::fill(workBuffer, workBuffer + DEFAULT_TUPLE_LEN * estimatedDigits, 0);
            multiplyTwice(getMatrixA(workBuffer, estimatedDigits), getMatrixB(workBuffer, estimatedDigits),
                          getMatrixA(fibMatrix, estimatedDigits),
                          getMatrixA(multiplierMatrix, estimatedDigits),
                          getMatrixB(multiplierMatrix, estimatedDigits),
                          currentFibLength, currentMultiplierLength);
            multiplyOnce(getMatrixA(workBuffer, estimatedDigits), getMatrixC(workBuffer, estimatedDigits),
                         getMatrixB(fibMatrix, estimatedDigits),
                         getMatrixB(multiplierMatrix, estimatedDigits),
                         currentFibLength, currentMultiplierLength);
            currentFibLength = multiplyTwice(getMatrixB(workBuffer, estimatedDigits), getMatrixC(workBuffer, estimatedDigits),
                                             getMatrixC(multiplierMatrix, estimatedDigits),
                                             getMatrixB(fibMatrix, estimatedDigits),
                                             getMatrixC(fibMatrix, estimatedDigits),
                                             currentMultiplierLength, currentFibLength);
            swapPointers(fibMatrix, workBuffer);
        }
        // Square the multiplier matrix.
        std::fill(workBuffer, workBuffer + DEFAULT_TUPLE_LEN * estimatedDigits, 0);
        multiplyTwice(getMatrixA(workBuffer, estimatedDigits), getMatrixB(workBuffer, estimatedDigits),
                      getMatrixA(multiplierMatrix, estimatedDigits),
                      getMatrixA(multiplierMatrix, estimatedDigits),
                      getMatrixB(multiplierMatrix, estimatedDigits),
                      currentMultiplierLength, currentMultiplierLength);
        multiplyOnce(getMatrixA(workBuffer, estimatedDigits), getMatrixC(workBuffer, estimatedDigits),
                     getMatrixB(multiplierMatrix, estimatedDigits),
                     getMatrixB(multiplierMatrix, estimatedDigits),
                     currentMultiplierLength, currentMultiplierLength);
        currentMultiplierLength = multiplyTwice(getMatrixB(workBuffer, estimatedDigits), getMatrixC(workBuffer, estimatedDigits),
                                               getMatrixC(multiplierMatrix, estimatedDigits),
                                               getMatrixB(multiplierMatrix, estimatedDigits),
                                               getMatrixC(multiplierMatrix, estimatedDigits),
                                               currentMultiplierLength, currentMultiplierLength);
        swapPointers(multiplierMatrix, workBuffer);
        fibIndex >>= 1;
    }
    
    // Build the result from the B block of fibMatrix.
    Number result;
    result.digits.resize(currentFibLength);
    std::copy(getMatrixB(fibMatrix, estimatedDigits),
              getMatrixB(fibMatrix, estimatedDigits) + currentFibLength,
              result.digits.begin());
    return result;
}

// Multithreaded Fibonacci computation wrapper.
// One-line comment: Runs fibonacci(index) in a separate thread and returns result if done before timeout.
Number fibonacci_mt(uint64_t index, std::chrono::milliseconds timeout) {
    std::future<Number> futureResult = std::async(std::launch::async, fibonacci, index);
    if (futureResult.wait_for(timeout) == std::future_status::ready) {
        return futureResult.get();
    } else {
        std::cerr << "fibonacci_mt: computation timed out." << std::endl;
        Number emptyResult;
        return emptyResult;
    }
}
