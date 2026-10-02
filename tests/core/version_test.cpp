#include <gtest/gtest.h>

#include "exsim/core/version.hpp"

TEST(Version, IsNotEmpty) {
    EXPECT_FALSE(exsim::version().empty());
}
