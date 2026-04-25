#include "Queue.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace {

TEST(QueueTest, DequeueReturnsSingleEnqueuedPayload) {
    Queue queue;
    Payload payload = 42;

    queue.enqueue(payload);

    EXPECT_EQ(queue.dequeue(), 42);
}

TEST(QueueTest, DequeueReturnsPayloadsInFirstInFirstOutOrder) {
    Queue queue;
    std::vector<Payload> payloads{1, 2, 3, 4, 5};

    for (Payload& payload : payloads) {
        queue.enqueue(payload);
    }

    for (Payload expected : payloads) {
        EXPECT_EQ(queue.dequeue(), expected);
    }
}

TEST(QueueTest, HandlesDuplicatePayloadsWithoutDroppingOrReorderingThem) {
    Queue queue;
    std::vector<Payload> payloads{7, 7, 3, 7, 3, 9};

    for (Payload& payload : payloads) {
        queue.enqueue(payload);
    }

    for (Payload expected : payloads) {
        EXPECT_EQ(queue.dequeue(), expected);
    }
}

TEST(QueueTest, HandlesNegativeZeroAndPositivePayloads) {
    Queue queue;
    std::vector<Payload> payloads{-10, 0, 25, -1, 100};

    for (Payload& payload : payloads) {
        queue.enqueue(payload);
    }

    for (Payload expected : payloads) {
        EXPECT_EQ(queue.dequeue(), expected);
    }
}

TEST(QueueTest, SupportsInterleavedEnqueueAndDequeueOperations) {
    Queue queue;
    Payload first = 10;
    Payload second = 20;
    Payload third = 30;
    Payload fourth = 40;

    queue.enqueue(first);
    queue.enqueue(second);
    EXPECT_EQ(queue.dequeue(), first);

    queue.enqueue(third);
    EXPECT_EQ(queue.dequeue(), second);

    queue.enqueue(fourth);
    EXPECT_EQ(queue.dequeue(), third);
    EXPECT_EQ(queue.dequeue(), fourth);
}

TEST(QueueTest, CanBeReusedAfterBecomingEmpty) {
    Queue queue;
    Payload first = 1;
    Payload second = 2;

    queue.enqueue(first);
    EXPECT_EQ(queue.dequeue(), first);

    queue.enqueue(second);
    EXPECT_EQ(queue.dequeue(), second);
}

TEST(QueueTest, MaintainsIndependentStateAcrossMultipleQueues) {
    Queue left;
    Queue right;
    Payload left_first = 1;
    Payload left_second = 2;
    Payload right_first = 100;
    Payload right_second = 200;

    left.enqueue(left_first);
    right.enqueue(right_first);
    left.enqueue(left_second);
    right.enqueue(right_second);

    EXPECT_EQ(left.dequeue(), left_first);
    EXPECT_EQ(left.dequeue(), left_second);
    EXPECT_EQ(right.dequeue(), right_first);
    EXPECT_EQ(right.dequeue(), right_second);
}

TEST(QueueTest, PreservesEveryPayloadInALargerSequence) {
    Queue queue;
    std::vector<Payload> payloads;

    for (int i = 0; i < 1000; ++i) {
        payloads.push_back(i);
    }

    for (Payload& payload : payloads) {
        queue.enqueue(payload);
    }

    for (Payload expected : payloads) {
        EXPECT_EQ(queue.dequeue(), expected);
    }
}

TEST(QueueTest, SupportsConcurrentProducers) {
    Queue queue;
    constexpr int producer_count = 4;
    constexpr int payloads_per_producer = 250;
    std::vector<std::thread> producers;

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&queue, producer] {
            for (int i = 0; i < payloads_per_producer; ++i) {
                Payload payload = producer * payloads_per_producer + i;
                queue.enqueue(payload);
            }
        });
    }

    for (std::thread& producer : producers) {
        producer.join();
    }

    std::vector<Payload> actual;
    actual.reserve(producer_count * payloads_per_producer);

    for (int i = 0; i < producer_count * payloads_per_producer; ++i) {
        actual.push_back(queue.dequeue());
    }

    std::sort(actual.begin(), actual.end());

    for (int expected = 0; expected < producer_count * payloads_per_producer; ++expected) {
        EXPECT_EQ(actual[expected], expected);
    }
}

TEST(QueueTest, SupportsOneProducerAndOneConsumerRunningConcurrently) {
    Queue queue;
    constexpr int payload_count = 1000;
    std::atomic<bool> producer_done{false};
    std::atomic<int> consumed_count{0};
    std::mutex consumed_mutex;
    std::vector<Payload> consumed;

    std::thread producer([&] {
        for (int i = 0; i < payload_count; ++i) {
            Payload payload = i;
            queue.enqueue(payload);
        }
        producer_done.store(true);
    });

    std::thread consumer([&] {
        while (consumed_count.load() < payload_count) {
            Payload payload = queue.dequeue();

            std::lock_guard<std::mutex> lock(consumed_mutex);
            consumed.push_back(payload);
            consumed_count.fetch_add(1);
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(consumed.size(), payload_count);
    for (int expected = 0; expected < payload_count; ++expected) {
        EXPECT_EQ(consumed[expected], expected);
    }
}

}  // namespace
