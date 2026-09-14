# Testing Nexum

Run every suite after building:

```sh
ctest --test-dir build-gcc-debug --output-on-failure
ctest --test-dir build-gcc-release --output-on-failure
```

For Visual Studio builds:

```bat
ctest --test-dir build-msvc -C Debug --output-on-failure
ctest --test-dir build-msvc -C Release --output-on-failure
```

The suites cover primitive validation and boundaries, graph queues and topology,
FIFO order, D1/D2 replay, conservative D3 analysis, expressivity compositions,
FSM and retry paths, generated/property-style cases, canonical v0 serialization,
cross-toolchain result vectors, the architecture freeze, Program IR, builder, and
compiler helpers.

## Sanitizers

Clang ASan:

```sh
cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Replace `address` with `undefined` and use `build-ubsan` for UBSan.

## Fuzzing

The Clang configuration builds the existing libFuzzer entry point:

```sh
cmake -S . -B build-fuzz -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DPNP_BUILD_FUZZ=ON
cmake --build build-fuzz
./build-fuzz/pnp_fuzz -runs=100000
```

Fuzzing exercises raw configuration validation and canonical config/state decoding.
A bounded run finding no crash is useful evidence, not proof of safety.

## Benchmarks

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/pnp_benchmark
```

Benchmarks are local performance indicators, not correctness gates or stable
performance guarantees. Do not commit generated benchmark output.
