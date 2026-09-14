# v0.1 Architecture Freeze Validation Report

## Verdict

`ARCHITECTURE FREEZE: PASS`

The current architecture is feature frozen as the v0.1 architecture candidate.
No new opcode, source selector, event destination, state-update mode, graph
semantic, or behavior-specific operation was introduced during validation.

## Validation changes

- Added exhaustive and torture coverage without changing the mathematical model.
- Enumerated every operand source, ALU operation, predicate, predicate inversion,
  state-update mode, event destination, event value source, and all 256 output masks.
- Added a critical D1 vector repeated 100,000 times.
- Expanded generated testing to 10,000 configurations across four fixed,
  reproducible seeds.
- Added graph topology, queue, execution-budget, serialization, shared-state,
  D3, FSM, retry, and expressivity regression coverage.
- Added retry input-order permutation tests.
- Extended the fuzz target to cover configuration and state decoding.
- Extended the benchmark harness with primitive, graph-chain, and retry cases.

One concrete defect was fixed: the ring queue tail calculation could wrap during
`uint32_t` addition at extreme capacities. The calculation is now overflow-safe,
and queue operations reject corrupted `head` or `count` values before indexing.

## Build and tests

- Debug build: PASS
- Release build: PASS
- GCC `-fanalyzer`: PASS
- Warning flags: `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`
- Project warnings: 0
- Debug CTest: 12/12 suites PASS
- Release CTest: 12/12 suites PASS

Coverage includes:

- primitive exhaustive boundaries: PASS
- validation torture: PASS
- generated/property tests: PASS
- D1 determinism: PASS
- D2 replay: PASS
- shared state domains: PASS
- conservative D3 analysis: PASS
- graph topology and FIFO ordering: PASS
- queue and execution budgets: PASS
- expressivity regression: PASS
- serialization torture: PASS
- ACK-driven retry, cancellation, retry limit, and failure path: PASS

## Expressivity cost

| Behavior | Primitives | Edges | Depth | Domains | State words | Executions/input | Max fan-out |
|---|---:|---:|---:|---:|---:|---:|---:|
| Pass | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Drop | 1 | 0 | 1 | 1 | 0 | 1 | 0 |
| Conditional drop | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Static route | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Conditional route | 1 | 2 | 1 | 1 | 0 | 1 | 1 |
| Fan-out | 1 | 2 | 1 | 1 | 0 | 1 | 2 |
| Broadcast | 1 | 8 | 1 | 1 | 0 | 1 | 8 |
| Metadata rewrite | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| TYPE rewrite | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| FLAGS rewrite | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Sequence rewrite | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Counter | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Conditional counter | 2 | 2 | 2 | 1 | 1 | 1-2 | 1 |
| Sequence validation | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Immediate dedup | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Threshold | 1 | 2 | 1 | 1 | 0 | 1 | 1 |
| Epoch counter | 4 | 5 | 4 | 1 | 2 | 2-4 | 1 |
| Toggle | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| FSM | 6 | 8 | 4 | 1 | 1 | 2-4 | 1 |
| ACK | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
| Timeout decision | 3 | 3 | 3 | 1 | 1 | 3 | 1 |
| Retry initialization | 4 | 4 | 4 | 1 | 3 | 4 | 1 |
| Retry | 11 | 13 | 6 | 1 | 3 | 5 | 1 |
| Retry cancellation | 11 | 13 | 6 | 1 | 3 | 4 | 1 |
| Retry failure path | 11 | 13 | 6 | 1 | 3 | 6 | 1 |
| Failover | 3 | 3 | 2 | 1 | 1 | 1-2 | 1 |
| Round-robin, 2 paths | 1 | 2 | 1 | 1 | 1 | 1 | 1 |

Retry is currently the most expensive and deepest demonstrated behavior at
11 primitives, 13 edges, and depth 6. The most frequently repeated composition
is event classification followed by a shared-state read/write and conditional
emission. Retry, epoch reset, and FSM construction have visible composition
cost, but no fundamental architectural blocker was found.

## Safety audits

### Heap allocation

No `malloc`, `calloc`, `realloc`, `free`, `strdup`, or Windows allocation API is
used by primitive execution, graph execution, queue operations, analysis, or
serialization. Runtime storage is caller-owned and preallocated.

### Forbidden features

Production execution contains no socket/network API, clock or time API, RNG,
callback dispatch, function-pointer extension mechanism, threads, locks,
atomics, sleep, filesystem I/O, or logging. Clock and printing functions are
confined to benchmark and test executables.

### Undefined behavior

- Arithmetic uses defined unsigned modulo behavior.
- Runtime shifts are masked to `0..63`.
- Invalid constant shifts are rejected by validation.
- Source, state, metadata, state-domain, node, edge, and output indexes are validated.
- Queue indexing rejects corrupted internal bounds.
- Serialization uses explicit little-endian encoding rather than raw C structs.
- Conversion of an event value into `TYPE` is an explicit `uint32_t` conversion.
- No further concrete undefined-behavior defect was found.

## Performance baseline

Measurements are Release wall-clock baselines from the validation machine and
are not cycle-accurate:

| Case | Result |
|---|---:|
| Primitive pass | 29 ns/op |
| ADD | 31 ns/op |
| XOR | 30 ns/op |
| Predicate true | 30 ns/op |
| Predicate false | 35 ns/op |
| State update | 32 ns/op |
| Event mutation | 31 ns/op |
| Maximum fan-out | 52 ns/op |
| Graph chain 4 | 230 ns/input |
| Graph chain 16 | 940 ns/input |
| Retry graph path | 240 ns/input |

## Unverified areas

- ASan: NOT VERIFIED. The available GCC toolchain lacks `libasan`; Clang could
  not link because the required Windows CRT libraries are unavailable.
- UBSan: NOT VERIFIED. The available GCC toolchain lacks `libubsan`; Clang had
  the same linker blocker.
- Fuzz target build: PASS.
- Fuzz runtime: NOT AVAILABLE.
- MSVC `/W4` build: not verified because the required MSVC CRT/toolchain was not available.

Sanitizer and fuzz-runtime availability do not currently constitute an
architecture failure, but they remain release-verification work.

## Architectural blockers

None identified during this validation round.

