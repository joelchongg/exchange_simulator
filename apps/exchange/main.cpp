#include <print>

#include "exsim/core/version.hpp"

int main() {
    std::println("exchange simulator {}", exsim::version());
    return 0;
}
