# v0.1 Cross-Toolchain Implementation Hardening Report

## Architecture

`FROZEN`

The mathematical primitive, opcodes, source selectors, event destinations,
state-domain ownership, graph semantics, and D1/D2/D3 rules were not changed.

## GCC

- Toolchain: GCC 13.3.0 on WSL2 Ubuntu 24.04
- Debug build: PASS
- Release build: PASS
- Debug tests: 13/13 PASS
- Release tests: 13/13 PASS
- Warning flags: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`
- Project warnings: 0

## Clang

- Toolchain: Clang 18.1.3 on WSL2 Ubuntu 24.04
- Debug build: PASS
- Release build: PASS
- Debug tests: 13/13 PASS
- Release tests: 13/13 PASS
- Warning flags: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`
- Project warnings: 0

## MSVC

- Toolchain: MSVC 19.42.34444
- Warning level: `/W4`
- Debug build: PASS
- Release build: PASS
- Debug tests: 13/13 PASS
- Release tests: 13/13 PASS
- Project warnings: 0

## Sanitizers

- AddressSanitizer: PASS
- UndefinedBehaviorSanitizer: PASS
- Toolchain: Clang 18.1.3 on WSL2 Ubuntu 24.04
- Test suite was actually executed under both sanitizers: 13/13 PASS
- Leak detection and halt-on-error were enabled.

## Fuzzing

- Clang/libFuzzer target build: PASS
- Runtime: PASS
- Executions: 100,000 bounded runs
- Maximum generated input length: 128 bytes
- Per-input timeout: 2 seconds
- Peak reported RSS: 46 MB
- Crashes: 0
- UB/OOB findings: 0
- Leak findings: 0

The fuzz target exercised configuration validation, canonical configuration
decoding, state decoding, and primitive execution for configurations accepted
by validation.

## Cross-toolchain determinism

`PASS`

The same fixed vector was executed with GCC, Clang, and MSVC. Each toolchain
produced an identical 628-byte canonical textual artifact containing:

- execution status,
- complete resulting state,
- canonical configuration serialization,
- output count and ports,
- every field of every emitted event.

Comparisons:

- MSVC vs GCC: identical
- MSVC vs Clang: identical
- GCC vs Clang: identical

No cross-toolchain difference was found.

## Regression fixes

### Explicit definition of `NULL`

Linux GCC detected that `pnp_validate.c` relied on an indirect definition of
`NULL`. The file now includes `<stddef.h>` explicitly.

### MSVC warning cleanliness

Three compile-time size and capacity checks in the primitive test produced
MSVC warning C4127. They were converted to C11 `_Static_assert` checks. The
final `/W4` Debug and Release builds contain zero project warnings.

### Runnable libFuzzer target

The existing portable fuzz source is now built as a runnable libFuzzer target
when Clang is selected. Other compilers retain the build-only object target.
This is tooling-only and does not affect production architecture or execution.

### Cross-toolchain artifact test

A deterministic test executable was added to emit a canonical representation
of primitive outputs, resulting state, and serialization for exact comparison
between toolchains.

## Linux Release performance baseline

Toolchain: GCC 13.3.0, WSL2 Ubuntu 24.04, Release build.

| Case | Result |
|---|---:|
| Primitive pass | 48.05 ns/op |
| ADD | 48.49 ns/op |
| XOR | 48.36 ns/op |
| Predicate true | 46.80 ns/op |
| Predicate false | 48.08 ns/op |
| State update | 48.50 ns/op |
| Event mutation | 48.83 ns/op |
| Maximum fan-out | 67.97 ns/op |
| Graph chain 4 | 300.78 ns/input |
| Graph chain 16 | 1,258.06 ns/input |
| Retry graph | 311.84 ns/input |

These are wall-clock baseline measurements, not cycle-accurate measurements.
No optimization or performance-driven refactoring was performed.

## Remaining unverified areas

No items remain unverified within the requested GCC, Clang, and MSVC x64
hardening matrix. ARM, MCU, and other target architectures were outside the
scope of this run.

## Final hardening verdict

`PASS`

All requested compiler matrices pass, warning counts are zero, sanitizer and
fuzzer runtimes were exercised successfully, and cross-toolchain results are
bit-identical. No architectural capability was added and no Git operation was
performed.

