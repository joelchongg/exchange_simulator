## Usage

```sh
cmake --preset debug          # or asan / tsan / release
cmake --build --preset debug
ctest --preset debug
```

Requires CMake ≥ 3.25 and Ninja. GoogleTest is fetched at configure time.
