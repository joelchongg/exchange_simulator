#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include "exsim/utils/seqlock_queue.hpp"

using exsim::utils::SeqLockQueue;
using exsim::utils::SeqLockQueueError;

namespace {

constexpr size_t kCapacity = 8;
using IntQueue = SeqLockQueue<int, kCapacity>;

}   // namespace

TEST(SeqLockQueue, PopOnEmptyQueueReturnsEmpty) {
    IntQueue q;
    auto reader = q.reader();

    int out = -1;
    auto res = reader.pop(out);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), SeqLockQueueError::Empty);
    EXPECT_EQ(out, -1);     // output untouched
}

TEST(SeqLockQueue, PushThenPopReturnsValue) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    writer.push(42);

    int out = 0;
    ASSERT_TRUE(reader.pop(out));
    EXPECT_EQ(out, 42);
}

TEST(SeqLockQueue, PopAfterDrainingReturnsEmpty) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    writer.push(1);
    int out = 0;
    ASSERT_TRUE(reader.pop(out));

    auto res = reader.pop(out);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), SeqLockQueueError::Empty);
}

TEST(SeqLockQueue, PreservesOrderAcrossWraparound) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    // interleave so the reader keeps up while the writer wraps several times
    for (int i = 0; i < static_cast<int>(kCapacity) * 5; ++i) {
        writer.push(i);
        int out = -1;
        ASSERT_TRUE(reader.pop(out)) << "at i=" << i;
        EXPECT_EQ(out, i);
    }
}

TEST(SeqLockQueue, FullBufferIsReadableWithoutOverrun) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    // Exactly Capacity pushes fill every slot once; nothing is overwritten yet.
    for (int i = 0; i < static_cast<int>(kCapacity); ++i) {
        writer.push(i);
    }
    for (int i = 0; i < static_cast<int>(kCapacity); ++i) {
        int out = -1;
        ASSERT_TRUE(reader.pop(out)) << "at i=" << i;
        EXPECT_EQ(out, i);
    }
}

TEST(SeqLockQueue, LappedReaderGetsOverrun) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    // One more than Capacity overwrites the slot the reader wants next.
    for (int i = 0; i < static_cast<int>(kCapacity) + 1; ++i) {
        writer.push(i);
    }

    int out = -1;
    auto res = reader.pop(out);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), SeqLockQueueError::Overrun);
}

TEST(SeqLockQueue, ReaderResyncsToWriterAfterOverrun) {
    IntQueue q;
    auto writer = q.writer();
    auto reader = q.reader();

    for (int i = 0; i < static_cast<int>(kCapacity) * 3; ++i) {
        writer.push(i);
    }

    int out = -1;
    auto res = reader.pop(out);
    ASSERT_FALSE(res);
    ASSERT_EQ(res.error(), SeqLockQueueError::Overrun);

    // resyncing should jump to the writer's position: nothing new until the next push
    res = reader.pop(out);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), SeqLockQueueError::Empty);

    writer.push(1000);
    ASSERT_TRUE(reader.pop(out));
    EXPECT_EQ(out, 1000);
}

TEST(SeqLockQueue, ReadersAreIndependent) {
    IntQueue q;
    auto writer = q.writer();
    auto fast = q.reader();
    auto slow = q.reader();

    for (int i = 0; i < 4; ++i) {
        writer.push(i);
    }

    // draining one reader should not consume anything from the other
    int out = -1;
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(fast.pop(out));
        EXPECT_EQ(out, i);
    }
    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(slow.pop(out));
        EXPECT_EQ(out, i);
    }
}

TEST(SeqLockQueue, WorksWithMultiWordElements) {
    struct Quote {
        uint64_t id;
        double price;
        uint32_t qty;
    };
    SeqLockQueue<Quote, kCapacity> q;
    auto writer = q.writer();
    auto reader = q.reader();

    writer.push(Quote{7, 101.25, 300});

    Quote out{};
    ASSERT_TRUE(reader.pop(out));
    EXPECT_EQ(out.id, 7u);
    EXPECT_EQ(out.price, 101.25);
    EXPECT_EQ(out.qty, 300u);
}

#ifndef NDEBUG
TEST(SeqLockQueueDeathTest, SecondWriterAsserts) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    IntQueue q;
    [[maybe_unused]] auto writer = q.writer();
    EXPECT_DEATH({ [[maybe_unused]] auto second = q.writer(); }, "");
}
#endif

// one writer, several readers, small capacity so the writer laps readers often.
// readers must also never see the counter go backwards.
TEST(SeqLockQueueStress, ConcurrentReadersNeverSeeTornOrReorderedData) {
    struct Payload {
        std::array<uint64_t, 8> words; // make payload larger than an atomic store
    };
    constexpr size_t kStressCapacity = 64;
    constexpr uint64_t kMessages = 500'000;
    constexpr size_t kReaders = 3;

    SeqLockQueue<Payload, kStressCapacity> q;
    std::atomic<bool> writer_done{false};
    std::atomic<int> readers_ready{0};

    std::atomic<uint64_t> torn{0};
    std::atomic<uint64_t> out_of_order{0};
    std::array<std::atomic<uint64_t>, kReaders> received{};

    std::vector<std::thread> readers;
    for (size_t r = 0; r < kReaders; ++r) {
        readers.emplace_back([&, r] {
            auto reader = q.reader();
            readers_ready.fetch_add(1, std::memory_order_release);

            uint64_t last = 0;
            bool seen_any = false;
            uint64_t count = 0;
            Payload out{};
            while (true) {
                auto res = reader.pop(out);
                if (res) {
                    const uint64_t v = out.words[0];
                    for (uint64_t w : out.words) {
                        if (w != v) {
                            torn.fetch_add(1, std::memory_order_relaxed);
                            break;
                        }
                    }
                    if (seen_any && v <= last) {
                        out_of_order.fetch_add(1, std::memory_order_relaxed);
                    }
                    last = v;
                    seen_any = true;
                    ++count;
                } else if (res.error() == SeqLockQueueError::Empty
                           && writer_done.load(std::memory_order_acquire)) {
                    break;
                }
            }
            received[r].store(count, std::memory_order_relaxed);
        });
    }

    while (readers_ready.load(std::memory_order_acquire) < static_cast<int>(kReaders)) {
        std::this_thread::yield();
    }

    {
        auto writer = q.writer();
        Payload p{};
        for (uint64_t i = 0; i < kMessages; ++i) {
            p.words.fill(i);
            writer.push(p);
        }
    }
    writer_done.store(true, std::memory_order_release);

    for (auto& t : readers) {
        t.join();
    }

    EXPECT_EQ(torn.load(), 0u);
    EXPECT_EQ(out_of_order.load(), 0u);
    for (size_t r = 0; r < kReaders; ++r) {
        EXPECT_GT(received[r].load(), 0u) << "reader " << r << " never read anything";
    }
}
