#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <new>
#include <type_traits>

#include "exsim/utils/errors.hpp"

namespace exsim {
namespace utils {

/*
 * The following SeqLockQueue implementation is based off seqlocks.
 * This allows for writers to be wait-free and readers to be lock-free.
 * Writers and readers track state on their own, instead of the queue tracking for them.
 */
template <typename ElemType, size_t Capacity>
requires (std::is_trivially_copyable_v<ElemType>)
class SeqLockQueue {
    static_assert(Capacity > 2 && (Capacity & (Capacity - 1)) == 0, "[SeqLockQueue] Capacity must be a power of 2 and greater than 2");

    struct alignas(std::hardware_destructive_interference_size) Slot {
        ElemType data;
        std::atomic<uint64_t> seq{0};
    };

public:
    class Writer {
    public:
        friend class SeqLockQueue;

        // disable copying
        Writer(const Writer&) = delete;
        Writer& operator=(const Writer&) = delete;

        // disable moving
        Writer(Writer&&) = delete;
        Writer& operator=(Writer&&) = delete;

        void push(const ElemType& elem) { q_.push(elem, seq_); }
    
    private:
        SeqLockQueue<ElemType, Capacity>& q_;
        uint64_t seq_;

        Writer(SeqLockQueue<ElemType, Capacity>& q, uint64_t seq)
            : q_(q), seq_(seq) {}
    };

    class Reader {
    public:
        friend class SeqLockQueue;

        // disable copying
        Reader(const Reader&) = delete;
        Reader& operator=(const Reader&) = delete;

        // disable moving
        Reader(Reader&&) = delete;
        Reader& operator=(Reader&&) = delete;

        std::expected<void, SeqLockQueueError> pop(ElemType& output) { return q_.pop(output, next_); }
    private:
        const SeqLockQueue<ElemType, Capacity>& q_;
        uint64_t next_;

        Reader(const SeqLockQueue<ElemType, Capacity>& q, uint64_t next)
            : q_(q), next_(next) {}
    };

    SeqLockQueue()
        : writer_pos_(0), buffer_(std::make_unique<Slot[]>(Capacity)), writer_created_(false) {}

    // disable copying
    SeqLockQueue(const SeqLockQueue&) = delete;
    SeqLockQueue& operator=(const SeqLockQueue<ElemType, Capacity>&) = delete;

    // disable moving
    SeqLockQueue(SeqLockQueue<ElemType, Capacity>&&) = delete;
    SeqLockQueue& operator=(SeqLockQueue<ElemType, Capacity>&&) = delete;

    // at most one writer per queue
    Writer writer() noexcept {
        assert(!writer_created_);
        writer_created_ = true;
        return Writer{*this, 0};
    }

    Reader reader() noexcept { return Reader{*this, 0}; }

private:
    alignas(std::hardware_destructive_interference_size) std::atomic<uint64_t> writer_pos_;
    alignas(std::hardware_destructive_interference_size) std::unique_ptr<Slot[]> buffer_;
    bool writer_created_;

    void push(const ElemType& elem, uint64_t& writer_gen) {
        uint64_t slot_idx = writer_gen & (Capacity - 1);
        Slot& slot = buffer_[slot_idx];
        slot.seq.store((writer_gen << 1) | 1, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
        slot.data = elem;
        slot.seq.store((writer_gen << 1) + 2, std::memory_order_release);
        ++writer_gen;
        writer_pos_.store(writer_gen, std::memory_order_relaxed);
    }

    std::expected<void, SeqLockQueueError> pop(ElemType& output, uint64_t& reader_gen) const {
        uint64_t slot_idx = reader_gen & (Capacity - 1);
        const Slot& slot = buffer_[slot_idx];
        
        while (true) {
            const uint64_t expected_seq = (reader_gen << 1) + 2;
            uint64_t curr_seq = slot.seq.load(std::memory_order_acquire);
            if (curr_seq < expected_seq) {
                // return instead of spinning
                return std::unexpected(SeqLockQueueError::Empty);
            } else if (curr_seq > expected_seq) {
                reader_gen = writer_pos_.load(std::memory_order_relaxed);
                return std::unexpected(SeqLockQueueError::Overrun);
            }

            output = slot.data;
            std::atomic_thread_fence(std::memory_order_acquire);
            if (slot.seq.load(std::memory_order_acquire) == curr_seq) {
                ++reader_gen;
                return {};
            }
        }
    }
};

}   // namespace utils
}   // namespace exsim
