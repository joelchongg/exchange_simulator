#pragma once

#include <cstdint>

namespace exsim {
namespace utils {

enum class SeqLockQueueError : uint8_t {
    Empty, Overrun
};

}   // namespace utils
}   // namespace exsim
