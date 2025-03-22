#include "fibonacci.h"
#include <cstdlib>
#include <cstring>
#include <climits>
#include <algorithm>
#include <future>
#include <chrono>
#include <iostream>

// This function returns a pointer to the first part (A) of our 3-tuple matrix.
// Basically, it gives us the starting location of the first section.
static inline DIGIT* getMatrixA(DIGIT* basePtr, size_t numDigits) {
    return basePtr;
}

// This function returns a pointer to the second part (B) of our matrix.
// We use this for storing part of the result.
static inline DIGIT* getMatrixB(DIGIT* basePtr, size_t numDigits) {
    return basePtr + numDigits;
}

// This function returns a pointer to the third part (C) of our matrix.
static inline DIGIT* getMatrixC(DIGIT* basePtr, size_t numDigits) {
    return basePtr + 2 * numDigits;
}

// This function estimates how many DIGITs are needed to store the Fibonacci number.
// I marked it as constexpr so that if we pass a constant, it can be computed at compile time.
static constexpr size_t estimateNumDigits(uint64_t fibIndex) {
    return (2 * fibIndex + DIGIT_BIT - 1) / DIGIT_BIT + 2;
}

// This function multiplies the array 'sourceDigits' by a multiplier and accumulates the result
// into two separate accumulators. It helps with big-number arithmetic.
static void scaleAndAccumulateOnce(DIGIT *accum1, DIGIT *accum2,
                                   const DIGIT *sourceDigits, DBDGT multiplier, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    // Loop over every digit in the array and perform multiplication and addition.
    for (i = 0; i < numDigits; ++i) {
        DBDGT product = ((DBDGT)sourceDigits[i]) * multiplier;
        DBDGT sum1 = ((DBDGT)accum1[i]) + product + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT; // get the carry for next digit

        DBDGT sum2 = ((DBDGT)accum2[i]) + product + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT;
    }
    // Store any remaining carry beyond the last digit.
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// This function does a similar job as scaleAndAccumulateOnce, but it uses two different multipliers
// for two accumulators at the same time.
static void scaleAndAccumulateTwice(DIGIT *accum1, DIGIT *accum2,
                                    const DIGIT *sourceDigits, DBDGT multiplier1, DBDGT multiplier2, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    // Loop through each digit and process both accumulations.
    for (i = 0; i < numDigits; ++i) {
        DBDGT sourceVal = sourceDigits[i];
        DBDGT sum1 = ((DBDGT)accum1[i]) + sourceVal * multiplier1 + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT;
        DBDGT sum2 = ((DBDGT)accum2[i]) + sourceVal * multiplier2 + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT;
    }
    // Handle carry over the top.
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// This function multiplies each digit of matrixB with matrixA and adds the result to accumulators.
// It basically loops over matrixB and calls scaleAndAccumulateOnce for each digit.
static void multiplyOnce(DIGIT *accum1, DIGIT *accum2,
                           const DIGIT *matrixA, const DIGIT *matrixB,
                           size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAndAccumulateOnce(&accum1[i], &accum2[i],
                                 matrixA, matrixB[i], numDigitsA);
    }
}

// This function multiplies matrixA with two multipliers (from another matrix) and accumulates the results.
// It returns the new length (number of digits) of the result.
static size_t multiplyTwice(DIGIT *accum1, DIGIT *accum2,
                            const DIGIT *matrixA, const DIGIT *multiplierA, const DIGIT *multiplierB,
                            size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAndAccumulateTwice(&accum1[i], &accum2[i],
                                matrixA, multiplierA[i], multiplierB[i], numDigitsA);
    }
    // Find the highest nonzero digit to determine the length of the result.
    size_t resultLength;
    for (resultLength = numDigitsA + numDigitsB; ; --resultLength) {
        if (accum1[resultLength] || accum2[resultLength]) {
            return resultLength + 1;
        }
    }
}

// This helper function swaps two pointers to DIGIT arrays.
static void swapPointers(DIGIT *&ptr1, DIGIT *&ptr2) {
    DIGIT* temp = ptr1;
    ptr1 = ptr2;
    ptr2 = temp;
}

// Main Fibonacci function using matrix exponentiation (3-tuple version).
// It computes Fibonacci numbers using the idea of raising a 2x2 matrix to a power.
Number fibonacci(uint64_t fibIndex) {
    // Estimate how many digits we need for the number.
    size_t estimatedDigits = estimateNumDigits(fibIndex);
    // Allocate one large block; we break it into three parts.
    size_t totalBlockSize = 3 * DEFAULT_TUPLE_LEN * estimatedDigits;
    std::vector<DIGIT> matrixBlock(totalBlockSize, 0);
    
    // Set pointers for the three sections: fibMatrix, multiplierMatrix, and workBuffer.
    DIGIT* fibMatrix = matrixBlock.data();
    DIGIT* multiplierMatrix = fibMatrix + DEFAULT_TUPLE_LEN * estimatedDigits;
    DIGIT* workBuffer = fibMatrix + 2 * DEFAULT_TUPLE_LEN * estimatedDigits;
    
    size_t currentFibLength = 1;
    size_t currentMultiplierLength = 1;
    
    // Initialize the fibMatrix as the identity matrix.
    // We set A=1, B=0, C=1.
    getMatrixA(fibMatrix, estimatedDigits)[0] = 1;
    getMatrixB(fibMatrix, estimatedDigits)[0] = 0;
    getMatrixC(fibMatrix, estimatedDigits)[0] = 1;
    
    // Initialize the multiplierMatrix to our Fibonacci matrix.
    // That is, we set A=0, B=1, C=1.
    getMatrixA(multiplierMatrix, estimatedDigits)[0] = 0;
    getMatrixB(multiplierMatrix, estimatedDigits)[0] = 1;
    getMatrixC(multiplierMatrix, estimatedDigits)[0] = 1;
    
    // Now we process each bit of the exponent (fibIndex).
    while (fibIndex) {
        if (fibIndex & 1) {
            // If the current bit is 1, update fibMatrix by multiplying it with multiplierMatrix.
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
            // Update the length based on the multiplication result.
            currentFibLength = multiplyTwice(getMatrixB(workBuffer, estimatedDigits), getMatrixC(workBuffer, estimatedDigits),
                                             getMatrixC(multiplierMatrix, estimatedDigits),
                                             getMatrixB(fibMatrix, estimatedDigits),
                                             getMatrixC(fibMatrix, estimatedDigits),
                                             currentMultiplierLength, currentFibLength);
            // Swap the fibMatrix with our workBuffer so the new value is stored.
            swapPointers(fibMatrix, workBuffer);
        }
        // Whether or not we multiplied, we need to square the multiplier matrix.
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
        // Swap multiplierMatrix with workBuffer.
        swapPointers(multiplierMatrix, workBuffer);
        // Shift the exponent right by one.
        fibIndex >>= 1;
    }
    
    // The final Fibonacci number is stored in the B block of fibMatrix.
    Number result;
    result.digits.resize(currentFibLength);
    std::copy(getMatrixB(fibMatrix, estimatedDigits),
              getMatrixB(fibMatrix, estimatedDigits) + currentFibLength,
              result.digits.begin());
    return result;
}

// This function wraps the fibonacci() call in a separate thread using std::async.
// It waits for the computation up to a given timeout and returns the result if ready.
Number fibonacci_mt(uint64_t index, std::chrono::milliseconds timeout) {
    // Launch the computation asynchronously.
    std::future<Number> futureResult = std::async(std::launch::async, fibonacci, index);
    // Wait for the result up to the timeout.
    if (futureResult.wait_for(timeout) == std::future_status::ready) {
        return futureResult.get();
    } else {
        std::cerr << "fibonacci_mt: computation timed out." << std::endl;
        Number emptyResult;
        return emptyResult;
    }
}
