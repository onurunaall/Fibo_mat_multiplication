#include "eval.h"
#include "fibonacci.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstdint>

// Computes Fibonacci for a given index, prints timing and size info,
// and returns the duration of the computation.
static std::chrono::nanoseconds computeAndPrint(uint64_t fibIndex) {
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    Number resultNumber = fibonacci(fibIndex);
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    size_t sizeInBytes = resultNumber.digits.size() * sizeof(DIGIT);
    std::cout << std::setw(20) << fibIndex << " | "
              << std::setw(3) << (duration.count() / 1000000000) << "."
              << std::setw(9) << (duration.count() % 1000000000)
              << "s | " << std::setw(6) << sizeInBytes << " B" << std::endl;
    return duration;
}

// Runs the evaluation loop for Fibonacci computation.
void runEvaluation() {
    const uint64_t FIRST_CHECKPOINT = 93;
    const uint64_t SECOND_CHECKPOINT = 0x2d7; // 727 in decimal
    const std::chrono::nanoseconds SOFT_CUTOFF(1500000000); // 1.5 sec
    const std::chrono::nanoseconds HARD_CUTOFF(1000000000);   // 1 sec

    uint64_t currentIndex = 0;
    uint64_t bestIndex = 0;

    std::cout << "#   Fibonacci index  |   Time (s)   | Size (bytes)" << std::endl;
    std::cout << "# -------------------+--------------+--------------" << std::endl;

    uint64_t idx;
    for (idx = currentIndex; idx <= FIRST_CHECKPOINT; ++idx) {
        std::chrono::nanoseconds duration = computeAndPrint(idx);
        if (duration > SOFT_CUTOFF) {
            break;
        }
        if (duration < HARD_CUTOFF) {
            bestIndex = idx;
        }
    }
    for (; idx <= SECOND_CHECKPOINT; ++idx) {
        std::chrono::nanoseconds duration = computeAndPrint(idx);
        if (duration > SOFT_CUTOFF) {
            break;
        }
        if (duration < HARD_CUTOFF) {
            bestIndex = idx;
        }
    }
    while (true) {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
        Number resultNumber = fibonacci(idx);
        std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        std::cout << std::setw(20) << idx << " | "
                  << std::setw(3) << (duration.count() / 1000000000) << "."
                  << std::setw(9) << (duration.count() % 1000000000)
                  << "s | " << std::setw(6)
                  << (resultNumber.digits.size() * sizeof(DIGIT)) << " B" << std::endl;
        if (duration > SOFT_CUTOFF) {
            break;
        }
        if (duration < HARD_CUTOFF) {
            bestIndex = idx;
        }
        uint64_t increment = (idx >> 1) - (idx >> 3);
        idx += increment;
        if (idx == 0) {
            idx = SECOND_CHECKPOINT + 1;
        }
    }
    std::cerr << "# Recorded best index: " << bestIndex << std::endl;
}
