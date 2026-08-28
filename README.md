# Lock-Free SPSC Ring Buffer for Low-Latency Systems

## Overview
This project is a high-performance, Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer written in C++20. 

Transitioning from an enterprise languages (Java/JVM) and distributed web systems to the microsecond-scale requirements of High-Frequency Trading (HFT), I built this queue to demonstrate strict mechanical sympathy. It bypasses operating system locks entirely, utilizing atomic hardware instructions and optimized memory layouts to achieve sub-microsecond cross-thread data transfer.

## Performance Benchmark
In a multi-threaded stress test transferring **1,000,000 sequentially ordered elements** between a producer thread and a consumer thread, the queue completes execution in **~0.06 seconds** (60 milliseconds) on an Apple Silicon architecture, ensuring strict FIFO ordering with zero data races.

## Architectural Highlights & Mechanical Sympathy

### 1. Bypassing the OS (Lock-Free Design)
Standard enterprise queues rely on OS-level Mutexes, incurring context-switch penalties of 2,000+ nanoseconds. This implementation uses `std::atomic` to allow threads to continuously spin and read/write directly from the CPU cache in ~1-5 nanoseconds without ever yielding to the operating system.

### 2. Hardware-Level Memory Ordering
To prevent out-of-order execution by the compiler or the CPU pipeline, the buffer enforces strict synchronisation barriers using `std::memory_order_acquire` and `std::memory_order_release`. This ensures data is fully visible in memory before the atomic index flags are updated.

### 3. Cache Line Padding (Preventing False Sharing)
In a naive implementation, the `write_index` and `read_index` share the same 64-byte L1 cache line, causing the producer and consumer CPU cores to constantly invalidate each other's cache (False Sharing). 
* **The Fix:** The atomic indices are explicitly aligned using `alignas(64)` to force them onto isolated cache lines, allowing both CPU cores to mutate their respective states with zero cross-core invalidation traffic.

### 4. Bitwise Wrap-Around
Modulo arithmetic (`%`) for circular buffer wrapping requires expensive integer division (15-40 CPU cycles). 
* **The Fix:** By enforcing a strict power-of-2 capacity via compile-time template metap rogramming (`static_assert`), the queue replaces modulo division with a single-cycle bitwise AND mask (`& (Capacity - 1)`).

## Building and Testing

The project uses CMake as its build system and automatically fetches Google Test for thread-safety and correctness verification.
- NOTE: You can write your own tests. I have used AI generated tests to check the project

### Prerequisites
* A C++20 compatible compiler (Clang/GCC)
* CMake (3.20+)

### Run Instructions
```bash
# 1. Clone the repository
git clone <YOUR_GITHUB_REPO_URL>
cd SPSCRingBuffer

# 2. Generate the build environment and fetch GTest
cmake -S . -B build

# 3. Compile the project
cmake --build build

# 4. Execute the concurrency and logic test suite
cd build && ctest --output-on-failure
