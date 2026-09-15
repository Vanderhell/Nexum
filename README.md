# Nexum

Nexum is an experimental programmable communication fabric built around a small,
deterministic execution primitive. It explores how routing, filtering, stateful
coordination, acknowledgements, retries, and similar communication behaviours can
be expressed by composing one bounded mathematical element rather than adding a
dedicated runtime operation for every behaviour.

**Status: Experimental / architecture frozen, implementation hardened.** Nexum is
research software. It is not production-ready, formally verified, a complete
network stack, a transport protocol, or a claim of vulnerability-free software.

## Model

The frozen primitive implements the transition:

```text
P_config(state, event) -> (next_state, bounded_outputs)
```

Configuration selects existing inputs, one ALU operation, a predicate, an optional
state update, an optional event-field update, and bounded output masks. Graphs
compose primitives through a caller-provided FIFO. Time and randomness are explicit
events rather than hidden environmental inputs.

```text
Compiler helpers / Program Builder
                |
                v
         Nexum Program IR
                |
                v
      deterministic lowering
                |
                v
       frozen PNP graph runtime
                |
                v
       primitive state transition
```

State is held in explicit, fixed-width state domains. Multiple cells may share a
domain only when the program says so. Program IR separates logical cell, domain,
edge, input, and output declarations from runtime array addresses and queue storage.
The builder assigns deterministic IDs and routing order without heap allocation;
the compiler helpers build several foundational compositions as syntactic sugar.

Nexum targets D1 primitive determinism and D2 ordered replay determinism. D3
schedule-independent determinism is a conservative graph-analysis result and is
not guaranteed for every graph. Queue capacity, output capacity, and execution
budgets remain explicit.

## Quick start

Prerequisites are a C11 compiler, CMake, and a supported build tool.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

A minimal compiler-helper flow is:

```c
#include "nexum_compile.h"

nexum_compile_spec_t spec = {0};
nexum_compile_requirements_t requirements;
nexum_program_t program;

spec.behavior = (uint8_t)NEXUM_BEHAVIOR_PASS;
if (nexum_compile_requirements(&spec, &requirements) != PNP_OK)
    return 1;
if (nexum_compile(&spec, requirements.capacities, &program) != PNP_OK)
    return 1;
```

The resulting `nexum_program_t` can be validated, queried for runtime storage
requirements, and lowered into caller-owned `pnp_graph_t` arrays. See
[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full path.

## Supported development toolchains

The repository is continuously intended to build with GCC and Clang on Ubuntu and
MSVC on Windows. Debug and Release configurations are tested. Exact commands are in
[`docs/BUILDING.md`](docs/BUILDING.md).

The CTest suite covers primitive semantics, graph topology and FIFO order,
deterministic replay, generated cases, serialization, expressivity, retry, the
architecture freeze, Program IR, builder, and compiler helpers. Clang ASan/UBSan,
the libFuzzer target, and benchmarks have separate workflows. See
[`docs/TESTING.md`](docs/TESTING.md).

## Zero-allocation boundary

Primitive and graph execution do not allocate memory. Program IR, builder,
compiler helpers, queues, graph arrays, state domains, and output buffers use
caller-owned or fixed bounded storage. This property reduces failure modes but is
not by itself a security proof.

## Repository layout

```text
include/       Public C headers
src/           Runtime, IR, builder, and compiler implementation
tests/         CTest suites
fuzz/          Portable/libFuzzer entry point
benchmarks/    Microbenchmark harness
docs/          Project documentation and policies
.github/       Contribution templates and automation
```

The historical technical specification is
[`programmable_network_primitive_SPEC.md`](programmable_network_primitive_SPEC.md).
The validated frozen baseline is recorded in
[`ARCHITECTURE_FREEZE_REPORT.md`](docs/reports/ARCHITECTURE_FREEZE_REPORT.md).

## Security

Nexum has undergone extensive tests and sanitizer runs, but testing does not prove
the absence of vulnerabilities. Do not use public issues for undisclosed security
reports. Follow [`SECURITY.md`](SECURITY.md) and read the technical
[`docs/SECURITY_MODEL.md`](docs/SECURITY_MODEL.md).

## Roadmap and contributions

Current and exploratory work are separated in [`docs/ROADMAP.md`](docs/ROADMAP.md).
Bounded event-local scratch now exists in internal queue envelopes, but primitives
cannot yet read or write it. A genuinely general declarative compiler therefore
remains research work. Optimizers, serialization formats, tracing, embedded
targets, SIMD, and hardware backends are later research—not committed APIs.

Contributions are welcome under [`CONTRIBUTING.md`](CONTRIBUTING.md) and
[`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md).

## License

Nexum is available under the Mozilla Public License 2.0. See [`LICENSE`](LICENSE).
