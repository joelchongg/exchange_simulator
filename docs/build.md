# Build setup

## Design note

- **Compilers:** GCC 14 and Clang 20 on Linux (ubuntu-24.04), and CI runs every preset with both. Apple Clang works for local development on macOS. Profiling and latency work happen on Linux only.
- **Standard:** full C++23 (`-std=c++23`, no extensions). This is why the floor is GCC 14 rather than 13: we want `std::print` and deducing `this`. Clang must be 19 or newer: with libstdc++, Clang 18 has neither `std::expected` (it reports `__cpp_concepts` as 201907) nor `std::hardware_destructive_interference_size`. Still unavailable at this floor: `std::flat_map` (GCC 15 / libc++ 20).
- **Warnings:** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion` plus a few more (see [cmake/warnings.cmake](../cmake/warnings.cmake)). `-Wconversion` stays on to catch implicit narrowing and widening between price, quantity and sequence-number types. `-Werror` is on in CI (`EXSIM_WARNINGS_AS_ERRORS=ON`) and off locally by default.
- **Presets:** `debug` (-O0 -g), `asan` (ASan+UBSan, -O1 -g, no recovery), `tsan` (TSan, -O1 -g), `release` (-O3 -g -DNDEBUG). `-march=native` is opt-in via `EXSIM_NATIVE=ON`.
- **Done when:** the trivial test passes in every preset, locally and in CI.

## Usage

```sh
cmake --preset debug          # or asan / tsan / release
cmake --build --preset debug
ctest --preset debug
```

Requires CMake ≥ 3.25 and Ninja. GoogleTest is fetched at configure time.
