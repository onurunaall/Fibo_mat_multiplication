#include "fibonacci.h"
#include <cstdlib>
#include <cstring>
#include <climits>
#include <vector>
#include <algorithm>

// For this alternate implementation, we use a 2-tuple matrix.
#define DEFAULT_TUPLE_LEN_2 2

// Use CHAR_BIT from <climits> to calculate bits per DIGIT.
static const int DIGIT_BIT_LOCAL = CHAR_BIT * sizeof(DIGIT);

// Estimates how many DIGITs are needed (alternate version) for the Fibonacci number.
static size_t estimateNumDigits2(uint64_t fibIndex) {
    return (2 * fibIndex + DIGIT_BIT_LOCAL - 1) / DIGIT_BIT_LOCAL + 2;
}

// Returns pointer to first block (A) for the 2-tuple matrix.
static DIGIT* getMatrixA2(DIGIT* basePtr, size_t numDigits) {
    return basePtr;
}

// Returns pointer to second block (B) for the 2-tuple matrix.
static DIGIT* getMatrixB2(DIGIT* basePtr, size_t numDigits) {
    return basePtr + numDigits;
}

// Multiplies source array by scale and accumulates into accum (alternate version).
static void scaleAccumulate(DIGIT *accum,
                            const DIGIT *sourceArray, DBDGT scale, size_t numDigits) {
    size_t i;
    DBDGT carry = 0;
    for (i = 0; i < numDigits; ++i) {
        DBDGT sourceValue = sourceArray[i];
        DBDGT sum = ((DBDGT)accum[i]) + sourceValue * scale + carry;
        accum[i] = (DIGIT)sum;
        carry = sum >> DIGIT_BIT_LOCAL;
    }
    DBDGT* extra = reinterpret_cast<DBDGT*>(&accum[numDigits]);
    *extra += carry;
}

// Multiplies source array by two scales and accumulates into accum1 and accum2 (alternate version).
static void scaleAccumulateTwice(DIGIT *accum1, DIGIT *accum2,
                                 const DIGIT *sourceArray, DBDGT scale1, DBDGT scale2, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    for (i = 0; i < numDigits; ++i) {
        DBDGT sourceValue = sourceArray[i];
        DBDGT sum1 = ((DBDGT)accum1[i]) + sourceValue * scale1 + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT_LOCAL;
        DBDGT sum2 = ((DBDGT)accum2[i]) + sourceValue * scale2 + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT_LOCAL;
    }
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// Multiplies source array by scale and accumulates into both accumulators (duplicate version).
static void scaleAccumulateDuplicate(DIGIT *accum1, DIGIT *accum2,
                                     const DIGIT *sourceArray, DBDGT scale, size_t numDigits) {
    size_t i;
    DBDGT carry1 = 0, carry2 = 0;
    for (i = 0; i < numDigits; ++i) {
        DBDGT product = ((DBDGT)sourceArray[i]) * scale;
        DBDGT sum1 = ((DBDGT)accum1[i]) + product + carry1;
        accum1[i] = (DIGIT)sum1;
        carry1 = sum1 >> DIGIT_BIT_LOCAL;
        DBDGT sum2 = ((DBDGT)accum2[i]) + product + carry2;
        accum2[i] = (DIGIT)sum2;
        carry2 = sum2 >> DIGIT_BIT_LOCAL;
    }
    DBDGT* extra1 = reinterpret_cast<DBDGT*>(&accum1[numDigits]);
    *extra1 += carry1;
    DBDGT* extra2 = reinterpret_cast<DBDGT*>(&accum2[numDigits]);
    *extra2 += carry2;
}

// Multiplies arrays a and b and accumulates the product into accum.
// Returns the number of digits of the result.
static size_t multiplyArrays(DIGIT *accum,
                             const DIGIT *a, const DIGIT *b,
                             size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAccumulate(&accum[i], a, b[i], numDigitsA);
    }
    size_t resultLength;
    for (resultLength = numDigitsA + numDigitsB; ; --resultLength) {
        if (accum[resultLength] != 0) {
            return resultLength + 1;
        }
    }
}

// Multiplies a1 by the pair (a2, b2) and accumulates the results in accum1 and accum2.
static void multiplyArraysTwice(DIGIT *accum1, DIGIT *accum2,
                                const DIGIT *a1, const DIGIT *a2, const DIGIT *b2,
                                size_t maxLength1, size_t maxLength2) {
    size_t i;
    for (i = 0; i < maxLength2; ++i) {
        scaleAccumulateTwice(&accum1[i], &accum2[i],
                             a1, a2[i], b2[i], maxLength1);
    }
}

// Multiplies arrays a and b and accumulates the product (duplicate version) into accum1 and accum2.
static void multiplyArraysDuplicate(DIGIT *accum1, DIGIT *accum2,
                                    const DIGIT *a, const DIGIT *b,
                                    size_t numDigitsA, size_t numDigitsB) {
    size_t i;
    for (i = 0; i < numDigitsB; ++i) {
        scaleAccumulateDuplicate(&accum1[i], &accum2[i],
                                 a, b[i], numDigitsA);
    }
}

// Swaps two pointers for the 2-tuple implementation.
static void swapPointers2(DIGIT **ptr1, DIGIT **ptr2) {
    DIGIT *temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

// Alternate Fibonacci computation using a 2-tuple matrix.
// Returns the computed Fibonacci number as a Number.
Number fibonacci2(uint64_t fibIndex) {
    size_t estimatedDigits = estimateNumDigits2(fibIndex);
    size_t totalBlockSize = 3 * DEFAULT_TUPLE_LEN_2 * estimatedDigits;
    std::vector<DIGIT> matrixBlock(totalBlockSize, 0);
    
    // Set up pointers for the 2-tuple matrices.
    DIGIT* fibMatrix = matrixBlock.data();
    DIGIT* multiplierMatrix = fibMatrix + DEFAULT_TUPLE_LEN_2 * estimatedDigits;
    DIGIT* workBuffer = fibMatrix + 2 * DEFAULT_TUPLE_LEN_2 * estimatedDigits;
    
    size_t currentFibLength = 1;
    size_t currentMultiplierLength = 1;
    
    // Initialize fibMatrix to identity: A2 = 1, B2 = 0.
    getMatrixA2(fibMatrix, estimatedDigits)[0] = 1;
    getMatrixB2(fibMatrix, estimatedDigits)[0] = 0;
    
    // Initialize multiplierMatrix to the Fibonacci matrix: A2 = 0, B2 = 1.
    getMatrixA2(multiplierMatrix, estimatedDigits)[0] = 0;
    getMatrixB2(multiplierMatrix, estimatedDigits)[0] = 1;
    
    while (fibIndex) {
        if (fibIndex & 1) {
            std::fill(workBuffer, workBuffer + DEFAULT_TUPLE_LEN_2 * estimatedDigits, 0);
            multiplyArraysTwice(getMatrixA2(workBuffer, estimatedDigits), getMatrixB2(workBuffer, estimatedDigits),
                                getMatrixA2(fibMatrix, estimatedDigits),
                                getMatrixA2(multiplierMatrix, estimatedDigits),
                                getMatrixB2(multiplierMatrix, estimatedDigits),
                                currentFibLength, currentMultiplierLength);
            multiplyArraysDuplicate(getMatrixA2(workBuffer, estimatedDigits), getMatrixB2(workBuffer, estimatedDigits),
                                    getMatrixB2(fibMatrix, estimatedDigits),
                                    getMatrixB2(multiplierMatrix, estimatedDigits),
                                    currentFibLength, currentMultiplierLength);
            currentFibLength = multiplyArrays(getMatrixB2(workBuffer, estimatedDigits),
                                              getMatrixB2(fibMatrix, estimatedDigits),
                                              getMatrixA2(multiplierMatrix, estimatedDigits),
                                              currentFibLength, currentMultiplierLength);
            swapPointers2(&fibMatrix, &workBuffer);
        }
        std::fill(workBuffer, workBuffer + DEFAULT_TUPLE_LEN_2 * estimatedDigits, 0);
        multiplyArraysTwice(getMatrixA2(workBuffer, estimatedDigits), getMatrixB2(workBuffer, estimatedDigits),
                            getMatrixA2(multiplierMatrix, estimatedDigits),
                            getMatrixA2(multiplierMatrix, estimatedDigits),
                            getMatrixB2(multiplierMatrix, estimatedDigits),
                            currentMultiplierLength, currentMultiplierLength);
        multiplyArraysDuplicate(getMatrixA2(workBuffer, estimatedDigits), getMatrixB2(workBuffer, estimatedDigits),
                                getMatrixB2(multiplierMatrix, estimatedDigits),
                                getMatrixB2(multiplierMatrix, estimatedDigits),
                                currentMultiplierLength, currentMultiplierLength);
        currentMultiplierLength = multiplyArrays(getMatrixB2(workBuffer, estimatedDigits),
                                                 getMatrixB2(multiplierMatrix, estimatedDigits),
                                                 getMatrixA2(multiplierMatrix, estimatedDigits),
                                                 currentMultiplierLength, currentMultiplierLength);
        swapPointers2(&multiplierMatrix, &workBuffer);
        fibIndex >>= 1;
    }
    Number result;
    result.digits.resize(currentFibLength);
    std::copy(getMatrixB2(fibMatrix, estimatedDigits),
              getMatrixB2(fibMatrix, estimatedDigits) + currentFibLength,
              result.digits.begin());
    return result;
}
