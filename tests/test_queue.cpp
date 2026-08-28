#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "SPSCQueue.h" // Assuming this is where you saved it

// ---------------------------------------------------------
// Test 1: Single-Threaded Correctness
// ---------------------------------------------------------
TEST(SPSCQueueTest, CapacityAndWrapAround) {
    // Create a queue with a strict capacity of 4
    SPSCQueue<int, 4> queue;
    int output = 0;

    // 1. Initial state
    EXPECT_FALSE(queue.pop(output)) << "Queue should be empty initially.";

    // 2. Fill the queue
    EXPECT_TRUE(queue.push(10));
    EXPECT_TRUE(queue.push(20));
    EXPECT_TRUE(queue.push(30));
    EXPECT_TRUE(queue.push(40));

    // 3. Queue is full, next push must fail
    EXPECT_FALSE(queue.push(50)) << "Queue should reject push when full.";

    // 4. Pop an item to make room (Read index advances)
    EXPECT_TRUE(queue.pop(output));
    EXPECT_EQ(output, 10);

    // 5. Wrap-around push (Write index wraps via bitwise AND)
    EXPECT_TRUE(queue.push(50)) << "Queue should accept push after making room.";
}

// ---------------------------------------------------------
// Test 2: Multi-Threaded Stress Test
// ---------------------------------------------------------
TEST(SPSCQueueTest, ConcurrentPushPop) {
    SPSCQueue<size_t, 1024> queue;
    const size_t NUM_MESSAGES = 1'000'000;

    // The Producer Thread
    std::thread producer([&]() {
        for (size_t i = 0; i < NUM_MESSAGES; ++i) {
            // "Spin" (busy-wait) if the queue is full
            while (!queue.push(i)) {}
        }
    });

    // The Consumer Thread
    std::thread consumer([&]() {
        for (size_t i = 0; i < NUM_MESSAGES; ++i) {
            size_t val;
            // "Spin" (busy-wait) if the queue is empty
            while (!queue.pop(val)) {} 
            
            // If the out-of-order execution logic (acquire/release) failed,
            // or if false sharing corrupted the indices, this check will fail.
            ASSERT_EQ(val, i) << "Data corruption or sequence loss detected!";
        }
    });

    // Wait for both threads to finish
    producer.join();
    consumer.join();
}