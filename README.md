# Fibonacci Matrix Exponentiation Project

This project implements variaty ways for Fibonacci number computation using matrix exponentiation in C++.

It contains two implementations:
- **fibonacci**: The original implementation using a 3‑tuple matrix.
- **fibonacci2**: An alternate implementation using a 2‑tuple matrix.

## Features
- Computes Fibonacci numbers with arbitrary precision.
- Checks system endianness.
- Outputs the result in hexadecimal format.
- Evaluates performance over increasing indices.

## How to Build
You can compile the project with a command like:

```bash
g++ -O2 -o fib_app main.cpp fibonacci.cpp fastexp2d.cpp utils.cpp eval.cpp
