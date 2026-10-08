include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    SYSTEM
    EXCLUDE_FROM_ALL)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(exsim_utils_tests
    utils/seqlock_queue_test.cpp)
target_link_libraries(exsim_utils_tests PRIVATE exsim::utils exsim::options GTest::gtest_main)
gtest_discover_tests(exsim_utils_tests DISCOVERY_MODE PRE_TEST)

add_executable(exsim_protocol_tests
    protocol/protocol_layout_test.cpp)
target_link_libraries(exsim_protocol_tests PRIVATE exsim::protocol exsim::options GTest::gtest_main)
gtest_discover_tests(exsim_protocol_tests DISCOVERY_MODE PRE_TEST)
