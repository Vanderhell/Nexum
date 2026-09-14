# Programmable Network Primitive
## Detailed Technical Specification for a Deterministic C Implementation

**Status:** Initial architecture specification  
**Language:** C  
**Primary implementation model:** AI-agent driven development (Codex)  
**Project type:** Experimental deterministic programmable networking core  
**Version:** 0.1-draft

---

# 1. Purpose

The goal of this project is to design and implement a **small, deterministic, programmable mathematical primitive** that can be composed into larger network behaviors.

The project is **not** a new network protocol.

The project is **not** a router, message broker, MQTT replacement, TCP replacement, SDN controller, or packet-processing framework.

The objective is to define a minimal mathematical execution element from which higher-level communication behaviors can be built by composition.

Examples of higher-level behaviors that should eventually be expressible without dedicated built-in operations include:

- filtering,
- routing,
- fan-out,
- multicast,
- broadcast,
- deduplication,
- sequence validation,
- counters,
- acknowledgements,
- retry logic,
- timeout behavior,
- failover,
- load balancing,
- rate limiting,
- windowing,
- aggregation,
- store-and-forward coordination,
- request/reply orchestration,
- priority handling.

These behaviors are **not primitives**. They are target behaviors used to test the expressive power of the primitive model.

---

# 2. Core Design Goal

The central design target is a primitive with the following abstract contract:

\[
P_\theta(S, E) \rightarrow (S', O)
\]

where:

- `θ` is immutable configuration,
- `S` is explicit bounded state,
- `E` is one explicit input event/token,
- `S'` is the next explicit state,
- `O` is a bounded set of output events.

The primitive must satisfy:

\[
|O| \le K
\]

where `K` is a compile-time or graph-build-time known maximum number of outputs.

The primitive must be:

- deterministic,
- bounded,
- replayable,
- side-effect free,
- small,
- cheap to execute,
- easy to serialize,
- easy to test,
- easy to compose,
- easy to map to CPU, MCU, SIMD, FPGA or ASIC-like execution models,
- suitable for static analysis,
- suitable for compiler optimization.

---

# 3. Non-Goals

The first implementation MUST NOT attempt to provide:

- sockets,
- TCP/UDP integration,
- network drivers,
- Wi-Fi,
- Ethernet,
- MQTT,
- HTTP,
- encryption,
- TLS,
- persistence,
- multithreading,
- distributed consensus,
- dynamic plugins,
- JIT compilation,
- scripting,
- garbage collection,
- runtime graph mutation,
- arbitrary user callbacks,
- arbitrary function pointers,
- operating-system specific integration.

Those may be explored later.

The first target is the **mathematical execution core only**.

---

# 4. Fundamental Rules

## 4.1 Determinism

For a given:

- configuration,
- state,
- input event,

the result MUST always be bit-identical.

Formally:

\[
P_\theta(S,E)=P_\theta(S,E)
\]

across repeated executions.

The result includes:

- next state,
- output count,
- output ports,
- output metadata,
- status code.

No hidden environmental information may influence the result.

---

## 4.2 No Hidden Inputs

The primitive MUST NOT directly access:

- system clock,
- wall clock,
- monotonic clock,
- random number generators,
- operating-system state,
- network state,
- global mutable variables,
- thread identity,
- process identity,
- filesystem,
- environment variables,
- sockets,
- hardware timers.

If time is needed, time MUST arrive as an input token.

If randomness is needed, a random value MUST arrive as an explicit input token.

If link status is needed, link state MUST arrive as an explicit input token.

Example:

Incorrect:

```c
if (clock_now() > deadline) {
    ...
}
```

Correct conceptual model:

```text
TIME_EVENT(t) -> primitive
```

---

## 4.3 No External Side Effects

The primitive MUST NOT:

- send packets,
- write files,
- allocate resources,
- lock mutexes,
- sleep,
- spawn threads,
- call callbacks,
- access a device,
- modify global state.

The primitive may only:

1. read immutable configuration,
2. read current state,
3. read one explicit input event,
4. compute,
5. produce next state,
6. produce bounded output tokens.

---

# 5. Determinism Levels

The project distinguishes three levels of determinism.

## D1 — Primitive Determinism

Same:

- configuration,
- state,
- input,

must produce the same output.

**Mandatory.**

---

## D2 — Replay Determinism

Given the same ordered input stream:

```text
E0, E1, E2, ... En
```

the graph must produce the same ordered output stream.

**Mandatory.**

---

## D3 — Schedule-Independent Determinism

Execution order, CPU scheduling, worker assignment, or timing differences must not alter the observable result.

**Not mandatory for every graph.**

D3 should eventually become an analyzable graph property.

The compiler/analyzer may later report:

```text
D1: PASS
D2: PASS
D3: PASS
```

or:

```text
D1: PASS
D2: PASS
D3: NOT PROVEN
```

The first implementation only needs D1 and D2.

---

# 6. Machine Word Model

The primitive operates on fixed-size machine words.

Recommended initial word type:

```c
typedef uint64_t pnp_word_t;
```

The abstract domain is:

\[
W = \mathbb{Z}/2^{64}\mathbb{Z}
\]

All integer arithmetic follows explicit modulo-2^64 behavior.

Unsigned integer arithmetic is preferred because C defines unsigned overflow behavior.

Signed overflow MUST NOT be used.

Floating-point arithmetic is forbidden in the initial implementation.

Reasons:

- deterministic bit-level behavior,
- portability,
- predictable overflow,
- easy SIMD mapping,
- easy hardware implementation,
- no floating-point environment differences,
- no NaN/rounding edge cases.

---

# 7. Input Event Model

The primitive receives one event.

The initial event representation should be fixed-size.

Example conceptual structure:

```c
typedef struct pnp_event {
    uint32_t type;
    uint32_t input_port;

    uint64_t sequence;
    uint64_t epoch;

    uint64_t fields[PNP_EVENT_FIELD_COUNT];

    uint64_t payload_ref;
    uint32_t payload_length;
    uint32_t flags;
} pnp_event_t;
```

This is only a starting model.

The exact layout MUST be benchmarked before being frozen.

Important principle:

> The primitive primarily operates on metadata, not arbitrary payload contents.

Payload should normally be represented by a reference, ID, index, offset, handle, or immutable buffer descriptor.

The primitive core MUST NOT copy arbitrary payload buffers in the hot path.

---

# 8. State Model

Each primitive owns a fixed-size explicit state.

Example:

```c
typedef struct pnp_state {
    pnp_word_t words[PNP_STATE_WORD_COUNT];
} pnp_state_t;
```

State properties:

- fixed size,
- known before execution,
- serializable,
- deterministic,
- no pointers to mutable external data,
- no hidden allocation,
- byte representation must be well-defined.

The first implementation should prefer a small number of state words.

Candidate starting point:

```text
4 to 16 x uint64_t
```

Do not freeze this value until benchmarks are available.

---

# 9. Configuration Model

Configuration is immutable during execution.

Example:

```c
typedef struct pnp_config {
    ...
} pnp_config_t;
```

Configuration may contain:

- opcode selection,
- source selectors,
- constants,
- masks,
- comparison mode,
- output mask mapping,
- state destination index,
- state update mode,
- event-field selection.

Configuration MUST NOT contain:

- mutable pointers,
- callbacks,
- function pointers,
- OS handles,
- locks,
- file descriptors.

The configuration should eventually be serializable to a compact binary form.

---

# 10. Primitive Execution Pipeline

The first candidate primitive architecture consists of four logical stages:

\[
COMPUTE \rightarrow DECIDE \rightarrow UPDATE \rightarrow EMIT
\]

Conceptually:

```text
                     configuration
                           |
                           v
                   +---------------+
event ------------>|    COMPUTE    |----> R
state ------------>|               |
                   +-------+-------+
                           |
                           v
                   +---------------+
                   |    DECIDE     |----> predicate/control
                   +-------+-------+
                           |
                  +--------+--------+
                  |                 |
                  v                 v
             +---------+       +---------+
             | UPDATE  |       |  EMIT   |
             +----+----+       +----+----+
                  |                 |
                  v                 v
              next state        outputs
```

These stages are conceptual.

The implementation MAY fuse them into a single function.

No ABI commitment is implied by the four-stage model.

---

# 11. COMPUTE Stage

The initial preferred model is a small word-based ALU.

Abstractly:

\[
R = ALU(A,B,OP)
\]

Where `A` and `B` may come from:

- event field,
- state word,
- constant,
- special zero value,
- special one value.

Candidate ALU operations:

```text
PASS_A
PASS_B
ADD
SUB
AND
OR
XOR
SHL
SHR
MIN_U
MAX_U
```

Potential later candidates:

```text
MUL
ROTATE_LEFT
ROTATE_RIGHT
POPCOUNT
CLZ
CTZ
BYTE_SWAP
SATURATING_ADD
```

These MUST NOT be added initially unless a real expressivity test requires them.

The initial implementation should prefer the smallest useful opcode set.

---

# 12. Operand Selection

Operands should be selected by compact source descriptors.

Example conceptual enum:

```c
typedef enum pnp_source_kind {
    PNP_SRC_ZERO,
    PNP_SRC_ONE,
    PNP_SRC_CONSTANT,
    PNP_SRC_EVENT_FIELD,
    PNP_SRC_STATE_WORD,
    PNP_SRC_SEQUENCE,
    PNP_SRC_EPOCH,
    PNP_SRC_FLAGS,
    PNP_SRC_INPUT_PORT
} pnp_source_kind_t;
```

A source selector may include:

```c
typedef struct pnp_source {
    uint8_t kind;
    uint8_t index;
    uint16_t reserved;
    uint64_t immediate;
} pnp_source_t;
```

The exact representation is not frozen.

Goal:

- low decode cost,
- no branches where possible,
- compact configuration,
- easy validation.

---

# 13. DECIDE Stage

The primitive produces a small predicate/control value.

Simplest initial form:

\[
G \in \{0,1\}
\]

Candidate comparisons:

```text
ALWAYS
NEVER
EQ
NE
LT_U
LE_U
GT_U
GE_U
BIT_SET
BIT_CLEAR
ZERO
NONZERO
```

However, the initial implementation should minimize redundant comparators.

For example:

- `NE` can be `EQ + invert`,
- `GT` can often be operand swap + `LT`,
- `GE` can be `LT + invert`.

A compact initial set may be:

```text
ALWAYS
EQ
LT_U
BIT_SET
```

plus an `invert` flag.

This should be validated experimentally.

---

# 14. UPDATE Stage

State update should be conditional.

Abstract form:

\[
S'_i =
\begin{cases}
V & G=1 \\
S_i & G=0
\end{cases}
\]

The primitive must support:

- no state write,
- write result,
- write operand,
- write selected event field,
- write constant.

The state destination index MUST be validated.

Initial implementation should allow at most one state-word write per primitive execution.

Multiple state writes may be added only if real use-cases prove the need.

Reason:

> A primitive should stay small. More complex state transitions can be expressed by chaining primitives.

---

# 15. EMIT Stage

The primitive does not perform network transmission.

It emits zero or more logical output events.

The simplest routing model is an output bitmask:

\[
M \in \{0,1\}^{K}
\]

Examples:

```text
0000 -> emit nothing
0001 -> output port 0
0010 -> output port 1
0011 -> output ports 0 and 1
1111 -> output ports 0,1,2,3
```

The output mask may depend on the predicate:

```text
mask_true
mask_false
```

Example:

```c
mask = predicate ? cfg->emit_true : cfg->emit_false;
```

This mechanism can express:

- drop,
- pass,
- fork,
- static route,
- conditional route,
- broadcast to known outputs.

No dedicated instructions for these behaviors should exist.

---

# 16. Event Mutation

The initial system should support limited deterministic event mutation.

Candidate model:

- each primitive may update zero or one event field before emission.

Example:

```text
field[2] = R
```

This enables graph composition without introducing arbitrary callbacks.

Possible source for event-field update:

- result `R`,
- state word,
- constant,
- operand A,
- operand B.

The mutation semantics MUST be explicitly defined.

For multi-output emission, all outputs should initially receive the same updated event value.

Per-output mutation should NOT be implemented in v0.1.

---

# 17. Initial Primitive Contract

A possible initial C-level execution contract:

```c
typedef enum pnp_result {
    PNP_OK = 0,
    PNP_ERR_INVALID_ARGUMENT,
    PNP_ERR_INVALID_CONFIG,
    PNP_ERR_INVALID_STATE,
    PNP_ERR_OUTPUT_OVERFLOW
} pnp_result_t;

typedef struct pnp_output {
    uint32_t port;
    pnp_event_t event;
} pnp_output_t;

typedef struct pnp_output_buffer {
    uint32_t count;
    pnp_output_t items[PNP_MAX_OUTPUTS];
} pnp_output_buffer_t;

pnp_result_t pnp_execute(
    const pnp_config_t *config,
    const pnp_state_t *state_in,
    const pnp_event_t *event_in,
    pnp_state_t *state_out,
    pnp_output_buffer_t *outputs
);
```

This API is not yet frozen.

Important requirements:

- no heap allocation,
- no global state,
- no static mutable state,
- no system calls,
- thread-safe if caller uses independent state/output objects,
- deterministic.

---

# 18. Error Semantics

Invalid configuration should preferably be rejected **before runtime execution**.

Create a validator:

```c
pnp_result_t pnp_validate_config(
    const pnp_config_t *config
);
```

Runtime should assume validated configuration where possible.

Two execution modes may later exist:

```text
checked
unchecked / validated-fast-path
```

The fast path must only be used after successful validation.

---

# 19. Graph Model

A program is a directed graph:

\[
G=(V,E)
\]

where:

- each vertex is one primitive instance,
- each edge connects an output port to another primitive input.

Conceptual model:

```text
                +---- P2 ----+
                |            |
INPUT -> P0 -> P1            P5 -> OUTPUT
                |            |
                +---- P3 -> P4
```

Every primitive uses the same execution contract.

There should not be separate runtime node classes for:

- filters,
- routers,
- retries,
- timers,
- aggregators.

Those must be graph behaviors.

---

# 20. Graph Execution Model

The first graph runtime should be:

- single-threaded,
- deterministic,
- bounded,
- queue-based,
- simple.

No parallel execution in v0.1.

Recommended event envelope:

```c
typedef struct pnp_graph_event {
    uint32_t node_id;
    uint32_t input_port;
    pnp_event_t event;
} pnp_graph_event_t;
```

Use a fixed-capacity ring buffer.

No heap allocation.

Example:

```text
external input
    |
    v
+-----------+
| event FIFO|
+-----+-----+
      |
      v
  primitive
      |
      +------ emitted graph events
```

---

# 21. Graph Queue Requirements

Queue capacity must be fixed before execution.

The graph runtime MUST detect queue overflow.

Queue overflow MUST NOT silently discard data.

Possible result:

```text
PNP_GRAPH_ERR_QUEUE_FULL
```

The analyzer should eventually estimate required queue capacity for bounded graphs.

---

# 22. Feedback

Graphs may contain cycles.

Example:

```text
      +-----------+
      |           |
      v           |
     P1 ---> P2 --+
```

However, cycles introduce risks:

- infinite event generation,
- unbounded queue growth,
- nontermination.

Therefore v0.1 should support cycles structurally but enforce an execution budget.

Example:

```c
typedef struct pnp_run_limits {
    uint64_t max_steps;
    uint64_t max_emitted_events;
} pnp_run_limits_t;
```

If the budget is exceeded:

```text
PNP_GRAPH_ERR_STEP_LIMIT
```

This is a safety mechanism, not the final mathematical solution.

Later versions should analyze graph boundedness statically.

---

# 23. Time Model

There is no implicit clock.

Time is data.

Example event:

```text
type = TIME
field[0] = logical_timestamp
```

Timeout behavior must therefore be expressible through state and explicit time events.

This guarantees replay.

A replay log containing identical:

```text
DATA
TIME
DATA
TIME
...
```

must produce identical results.

---

# 24. Randomness Model

There is no internal RNG.

Randomness must be injected explicitly:

```text
type = RANDOM
field[0] = random_value
```

This allows deterministic replay.

The production system may use an external random source, but the primitive core does not know where the value originated.

---

# 25. Payload Model

Payload data is conceptually outside the primitive core.

The event contains:

```text
payload_ref
payload_length
```

The primitive may:

- forward the reference,
- compare length,
- replace the reference using explicit deterministic data,
- potentially select metadata based on length.

The primitive MUST NOT dereference arbitrary payload memory in v0.1.

This prevents:

- memory-safety issues,
- unpredictable cost,
- architecture-dependent behavior,
- cache-dependent complexity in the core design.

Payload inspection may become a separate explicit primitive class later if justified.

---

# 26. Required Expressivity Test Suite

The architecture is not considered useful until the primitive graph can express the following behaviors.

These are **tests**, not built-in features.

## Tier 1 — Basic

1. Pass-through
2. Drop
3. Conditional drop
4. Static route
5. Conditional route
6. Fan-out
7. Broadcast
8. Metadata rewrite
9. Counter
10. Conditional counter

---

## Tier 2 — Stateful

11. Sequence checking
12. Duplicate suppression
13. Threshold detection
14. Event counting per epoch
15. Toggle behavior
16. Finite-state machine
17. Priority classification
18. Simple rate counter

---

## Tier 3 — Graph Behaviors

19. ACK generation
20. Retry state machine
21. Timeout via explicit time events
22. Failover path selection
23. Round-robin routing
24. Weighted routing using explicit deterministic inputs
25. Request/reply correlation
26. Store-forward control signaling
27. Window-open/window-close state
28. Basic aggregation trigger

---

## Tier 4 — Advanced

29. Bounded sliding-window logic
30. Token bucket rate limiting
31. Deduplication window
32. Multi-path forwarding
33. Deterministic load distribution
34. Sequence reorder detection
35. Flow-state machine
36. Epoch transition handling

If any of these require adding a dedicated opcode named after the behavior, the architecture should be reconsidered first.

---

# 27. Anti-Patterns

The implementation MUST NOT add operations such as:

```text
ROUTE
FILTER
RETRY
ACK
BROADCAST
MULTICAST
DEDUP
TIMEOUT
FAILOVER
LOAD_BALANCE
RATE_LIMIT
```

These are high-level behaviors.

The architecture should express them through:

- ALU operations,
- predicates,
- state,
- event mutation,
- output masks,
- graph topology,
- explicit input events.

---

# 28. Performance Objectives

The first version does not require final production performance.

However, the design must target:

- zero heap allocations in `pnp_execute`,
- constant bounded memory,
- low branch count,
- compact configuration,
- simple data structures,
- predictable CPU behavior,
- no virtual dispatch,
- no function-pointer dispatch in hot path,
- cache-friendly layouts,
- no recursion.

Potential later optimization areas:

- branchless execution,
- lookup-table opcode dispatch,
- computed goto where portable strategy allows,
- SIMD batching,
- structure-of-arrays graph execution,
- primitive fusion,
- constant propagation,
- dead-node elimination.

Do not optimize prematurely.

Correctness and determinism come first.

---

# 29. Complexity Requirements

For each primitive configuration, the following should eventually be statically derivable:

```text
state_size
config_size
max_outputs
max_event_mutations
max_execution_steps
```

For a graph:

```text
primitive_count
edge_count
state_memory
queue_memory
max_fanout
maximum configured execution budget
```

Long-term objective:

\[
M_{max}
\]

maximum required memory should be known before graph execution.

---

# 30. Serialization

The project should eventually support stable binary serialization for:

- configuration,
- primitive state,
- graph topology,
- graph state.

However:

**Do not design a public stable binary ABI in the first implementation.**

First use internal in-memory structures.

Once semantics are stable, define binary serialization separately.

Do not couple v0.1 implementation structs directly to permanent wire formats.

---

# 31. Portability

Target baseline:

```text
C11
```

Potential platforms:

- Windows x64,
- Linux x64,
- Linux ARM64,
- ESP32-class MCU later,
- STM32-class MCU later.

Initial development platforms:

- Windows,
- Linux.

Rules:

- standard fixed-width integer types,
- no compiler-specific behavior in core logic,
- no undefined behavior,
- no signed overflow,
- no invalid aliasing,
- no unaligned access assumptions,
- no endianness assumptions in serialization code.

---

# 32. Memory Policy

Core rule:

> No dynamic allocation in primitive execution.

For v0.1 it is acceptable to use allocation during graph construction if isolated behind a builder.

Preferred eventual model:

- caller-provided storage,
- fixed-capacity arenas,
- fixed-size arrays.

The execution runtime should operate entirely on preallocated memory.

---

# 33. Threading Policy

v0.1:

```text
single-threaded only
```

The code should still avoid global mutable state so that multiple independent runtimes can execute concurrently in separate threads.

Do not add locks.

Do not add atomics unless a specific requirement appears.

D3 parallel determinism belongs to a later phase.

---

# 34. Logging Policy

The mathematical core should not log.

No:

```c
printf()
fprintf()
OutputDebugString()
syslog()
```

in execution code.

Diagnostics should be returned through status/error structures.

Test binaries may print results.

---

# 35. Naming Guidance

Working prefix:

```text
pnp_
```

Meaning:

```text
Programmable Network Primitive
```

This is only a temporary internal namespace.

Do not spend implementation time on branding before the mathematical model is validated.

---

# 36. Repository Layout

Recommended initial layout:

```text
/
├─ CMakeLists.txt
├─ README.md
├─ SPEC.md
├─ include/
│  ├─ pnp_types.h
│  ├─ pnp_primitive.h
│  ├─ pnp_graph.h
│  └─ pnp_result.h
│
├─ src/
│  ├─ pnp_primitive.c
│  ├─ pnp_validate.c
│  ├─ pnp_graph.c
│  └─ pnp_internal.h
│
├─ tests/
│  ├─ test_primitive.c
│  ├─ test_determinism.c
│  ├─ test_graph.c
│  ├─ test_expressivity_basic.c
│  ├─ test_expressivity_stateful.c
│  └─ test_expressivity_graph.c
│
├─ examples/
│  ├─ simple_filter.c
│  ├─ conditional_route.c
│  └─ dedup.c
│
└─ tools/
```

Do not introduce unnecessary libraries or directory layers.

---

# 37. Build System

Use:

```text
CMake
```

Requirements:

- clean Debug build,
- clean Release build,
- warnings enabled,
- warnings treated as errors in CI where practical.

Recommended warning levels:

## MSVC

```text
/W4
```

## GCC/Clang

```text
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wshadow
```

Be pragmatic with warnings that reduce portability or generate noise.

---

# 38. Testing Requirements

Every implementation phase MUST include tests.

No feature is complete without tests.

## Unit tests

Test:

- every ALU opcode,
- every comparator,
- every source selector,
- every state update mode,
- output-mask behavior,
- event mutation,
- invalid configuration,
- boundary indexes,
- zero values,
- max values,
- overflow behavior.

---

## Determinism tests

For each test vector:

1. initialize same config,
2. initialize same state,
3. execute same event many times from identical starting state,
4. compare all bytes of:
   - next state,
   - output count,
   - outputs.

Recommended repetition:

```text
10,000+
```

for deterministic stress tests.

---

## Replay tests

Create an input stream.

Run the graph.

Save outputs.

Reset all graph state.

Replay exactly the same input stream.

Outputs and final state MUST be byte-identical.

---

# 39. Property-Based / Generated Tests

A later phase should generate random valid primitive configurations and states.

Test invariant:

```text
execute(config, state, event)
```

must produce the same result across repeated runs.

Generated tests should also verify:

- no out-of-bounds writes,
- output count <= K,
- state only changes according to declared update semantics.

---

# 40. Sanitizers and Dynamic Analysis

Linux/Clang or GCC builds should eventually run:

- AddressSanitizer,
- UndefinedBehaviorSanitizer.

Recommended:

```text
-fsanitize=address,undefined
```

Valgrind may be added later.

No memory errors are acceptable.

---

# 41. Fuzzing

Once parser/serialization/config validation exists, fuzz:

- configuration validator,
- graph loader,
- primitive execution with validated random configs.

Potential tooling:

- libFuzzer,
- AFL++.

Do not fuzz invalid raw pointers or undefined host memory.

---

# 42. Benchmarking

Create benchmarks only after semantics stabilize.

Measure:

```text
ns / primitive execution
executions / second
cycles / primitive
branch misses
cache misses
graph events / second
```

Benchmark classes:

1. stateless pass-through,
2. ALU only,
3. compare,
4. state update,
5. single emit,
6. fan-out,
7. graph chain,
8. graph branch,
9. graph feedback.

Do not optimize based on synthetic benchmarks alone.

---

# 43. Initial Development Phases

## Phase 0 — Skeleton

Deliver:

- repository structure,
- CMake,
- basic types,
- empty tests,
- CI-ready build.

No network integration.

Acceptance:

```text
Debug build PASS
Release build PASS
tests PASS
```

---

## Phase 1 — Word ALU

Implement:

- source selection,
- minimal ALU operations,
- deterministic result.

Suggested initial operations:

```text
PASS_A
ADD
SUB
AND
OR
XOR
SHL
SHR
```

Acceptance:

- full opcode unit tests,
- overflow behavior tests,
- deterministic replay of primitive calls.

---

## Phase 2 — Predicate

Implement minimal predicate model.

Suggested:

```text
ALWAYS
EQ
LT_U
BIT_SET
```

with optional inversion.

Acceptance:

- all comparator edge cases,
- max/min values,
- bit 0 and bit 63,
- invalid bit index rejected by validator.

---

## Phase 3 — State Update

Implement:

- zero or one state write,
- conditional write,
- configurable destination.

Acceptance:

- untouched state remains byte-identical,
- invalid index rejected,
- update deterministic.

---

## Phase 4 — Emit

Implement:

- output mask,
- zero outputs,
- one output,
- multiple outputs,
- bounded output count.

Acceptance:

```text
count <= PNP_MAX_OUTPUTS
```

always.

---

## Phase 5 — Event Mutation

Implement:

- optional one-field rewrite,
- deterministic multi-output copying.

Acceptance:

- original input event is never mutated,
- all emitted outputs have defined contents.

---

## Phase 6 — Config Validator

Implement complete static validation.

After validation, `pnp_execute` should be able to use a simpler fast path.

Do not prematurely split APIs until measured.

---

## Phase 7 — Graph Runtime

Implement:

- fixed nodes,
- fixed edges,
- fixed queue,
- event dispatch,
- state storage per node,
- deterministic single-thread scheduling.

Acceptance:

- chain,
- branch,
- merge,
- fan-out,
- cycle with explicit step limit.

---

## Phase 8 — Expressivity Tier 1

Implement tests only.

Do not add high-level runtime features.

Prove graph configurations for:

- pass,
- drop,
- conditional drop,
- route,
- fan-out,
- broadcast,
- metadata rewrite,
- counter.

If a required behavior cannot be expressed, document why before changing the primitive.

---

## Phase 9 — Expressivity Tier 2

Test:

- sequence validation,
- dedup,
- threshold,
- finite-state machine,
- priority classification.

Again:

> Do not add behavior-specific opcodes.

---

## Phase 10 — Time and Feedback

Introduce explicit time-event conventions.

Build:

- timeout graph,
- retry state machine.

Do not add:

```text
TIMEOUT opcode
RETRY opcode
```

---

# 44. Change-Control Rule

The mathematical contract is more important than implementation convenience.

An AI agent MUST NOT add:

- new opcodes,
- new state mutation modes,
- new event fields,
- new graph semantics,

merely because a test is difficult to express.

Before extending the model, the agent must document:

1. what behavior cannot be represented,
2. why existing composition is insufficient,
3. the minimal extension proposed,
4. whether the extension can be derived from existing operations,
5. impact on determinism,
6. impact on execution bounds,
7. impact on configuration size,
8. impact on portability.

This is mandatory.

---

# 45. AI-Agent Implementation Rules

The implementation will be driven by AI agents.

Agents must follow the specification literally.

## Agent MUST

- inspect existing code before modifying it,
- preserve deterministic semantics,
- implement the smallest required change,
- add or update tests for every behavior,
- run the relevant test suite,
- report exact build/test status,
- explicitly report unverified items,
- avoid speculative abstractions,
- avoid unrelated refactoring.

---

## Agent MUST NOT

- redesign architecture without explicit instruction,
- rename public concepts casually,
- add hidden global state,
- add dynamic allocation to the execution path,
- introduce C++,
- add third-party dependencies without approval,
- add network protocols,
- add background threads,
- add callbacks into execution,
- add function-pointer extensibility,
- use random or time APIs,
- suppress failing tests,
- weaken assertions merely to pass CI,
- mark incomplete tests as successful.

---

# 46. Agent Reporting Format

After each implementation task, the agent should report:

```text
Implemented:
- ...

Changed files:
- ...

Build:
- Debug: PASS/FAIL
- Release: PASS/FAIL

Tests:
- <suite>: PASS/FAIL
- total: X/Y

Sanitizers:
- PASS/FAIL/NOT RUN

Unverified:
- ...

Remaining issues:
- ...
```

No vague statements such as:

```text
should work
probably fixed
looks correct
```

without test evidence.

---

# 47. Git Rules for AI Agents

Agents must not:

- create commits unless explicitly requested,
- push unless explicitly requested,
- rewrite history,
- force push,
- change authorship metadata,
- add themselves as Author,
- add themselves as Co-Author.

Commit authorship belongs to the user.

---

# 48. Coding Style

Initial rules:

- plain C,
- explicit types,
- short execution functions,
- no macro metaprogramming unless justified,
- no opaque magic constants,
- no clever undefined-behavior tricks,
- no hidden ownership,
- no unnecessary typedef layers,
- internal helper functions should generally be `static`.

Prefer clarity in v0.1.

Optimization happens after behavioral validation.

---

# 49. Header/API Discipline

Public headers should expose only concepts required by callers.

Internal details belong in:

```text
src/pnp_internal.h
```

Do not expose:

- internal queue layout,
- validation helper state,
- benchmark-only details,
- private execution intermediates.

---

# 50. Security Mindset

Although v0.1 is not a security product, all external configuration must be treated as potentially invalid.

Validate:

- enum ranges,
- indexes,
- output masks,
- state indexes,
- event-field indexes,
- shift counts,
- bit indexes,
- graph node IDs,
- edge targets,
- queue capacities,
- integer size conversions.

No unchecked indexing.

---

# 51. Undefined Behavior Policy

Undefined behavior is forbidden.

Particular care:

- shifts >= word width,
- signed overflow,
- out-of-bounds access,
- null dereference,
- invalid enum assumptions,
- uninitialized padding comparisons,
- strict aliasing,
- pointer arithmetic.

If byte-identical state comparisons are required, structs containing padding must be handled carefully.

Prefer explicit field comparison or zero-initialized storage.

---

# 52. Open Questions

The following are intentionally unresolved.

## Q1 — Exact event size

Possible options:

- 64 bytes,
- 96 bytes,
- 128 bytes,
- configurable compile-time size.

Must be benchmarked.

---

## Q2 — Exact state size

Possible:

- fixed global count,
- per-node declared count,
- compact inline state.

Prefer per-node bounded state eventually.

---

## Q3 — Output limit K

Candidate starting values:

```text
4
8
16
```

A smaller value improves compactness.

A larger value improves direct fan-out.

Do not decide without expressivity and benchmark data.

---

## Q4 — One ALU operation per primitive?

Current preferred answer:

```text
yes
```

Reason:

- small cell,
- easy composition,
- easy optimization,
- FPGA-like model.

But benchmark graph-depth cost before freezing.

---

## Q5 — One state write per primitive?

Current preferred answer:

```text
yes
```

Same reason.

---

## Q6 — One event-field write per primitive?

Current preferred answer:

```text
yes
```

Must be validated.

---

## Q7 — Per-output mutation?

Current preferred answer:

```text
no in v0.1
```

Would significantly complicate the primitive.

---

## Q8 — Multiple predicates?

Current preferred answer:

```text
one predicate
```

Boolean networks should be composed from multiple primitives unless this becomes prohibitively expensive.

---

# 53. Architecture Evaluation Criteria

After Tier 1 and Tier 2 expressivity tests, evaluate the design using:

## Minimality

How many primitive features are required?

## Expressivity

How many target behaviors can be built without special opcodes?

## Cost

How many primitive cells are required for common behavior?

## Memory

How much state/configuration is required?

## Predictability

Can execution bounds be calculated?

## Replay

Is byte-identical replay reliable?

## Optimization potential

Can chains be fused or simplified later?

## Hardware mapping

Can the primitive map naturally onto:

- CPU,
- SIMD,
- MCU,
- FPGA?

---

# 54. Long-Term Compiler Model

The long-term toolchain may become:

```text
high-level behavior specification
            |
            v
      graph compiler
            |
            v
      primitive IR
            |
            v
        optimizer
            |
            v
       graph mapper
            |
      +-----+------+
      |     |      |
      v     v      v
     CPU   MCU    FPGA
```

This is not part of v0.1 implementation.

However, the primitive representation should not prevent this future direction.

---

# 55. Long-Term Optimizations

Potential compiler optimizations:

- constant folding,
- dead primitive elimination,
- predicate simplification,
- state-write elimination,
- consecutive ALU fusion,
- output-mask simplification,
- graph inlining,
- graph specialization,
- static route resolution,
- common subexpression elimination where state semantics permit,
- SIMD batching.

Do not implement any of these before correctness is established.

---

# 56. Fundamental Philosophy

The project should preserve the following principle:

> Complexity belongs in composition, not in the primitive.

A primitive should not know that it is implementing:

- retry,
- routing,
- telemetry,
- ACK,
- load balancing,
- failover.

It only knows:

```text
read
compute
decide
update
emit
```

The graph creates the higher-level meaning.

---

# 57. Initial Mathematical Candidate

The current candidate model is:

\[
R = ALU_\theta(A(X,S),B(X,S))
\]

\[
G = Predicate_\theta(R,X,S)
\]

\[
S' = Update_\theta(S,R,G)
\]

\[
O = Emit_\theta(X,S,R,G)
\]

subject to:

\[
|O| \le K
\]

and all memory and execution bounds being finite and known.

This model is **not yet considered final**.

The implementation exists to validate or falsify it.

That distinction is critical.

---

# 58. Success Criterion for v0.1

v0.1 is successful if:

1. the primitive is deterministic,
2. execution uses no heap allocation,
3. execution uses no external side effects,
4. state is explicit,
5. configuration is explicit,
6. graph runtime is deterministic,
7. Tier 1 expressivity passes,
8. Tier 2 expressivity passes substantially,
9. replay tests are byte-identical,
10. no behavior-specific opcodes were required,
11. sanitizer runs show no memory/UB defects,
12. implementation remains small enough to understand completely.

Performance numbers are secondary at this stage.

---

# 59. Failure Criterion

The current primitive model should be reconsidered if:

- common behaviors require excessive graph depth,
- configuration is larger than the behavior it replaces,
- state becomes difficult to bound,
- graph execution requires hidden runtime features,
- dedicated high-level opcodes keep being added,
- deterministic replay becomes complicated,
- simple behaviors require dozens/hundreds of primitives,
- primitive execution becomes branch-heavy or opaque,
- the model cannot represent time/feedback cleanly.

Failure of the current model is acceptable.

The purpose of the early implementation is to test the model scientifically, not to defend it.

---

# 60. First Codex Task

The first implementation task should be intentionally small.

## Objective

Create only the project skeleton and core type contracts.

## Implement

- CMake project,
- C11 configuration,
- public result enum,
- fixed-width word type,
- preliminary event structure,
- preliminary state structure,
- preliminary config structure,
- stub config validator,
- stub primitive execute function,
- initial unit-test executable.

## Do not implement yet

- graph runtime,
- ALU behavior,
- predicates,
- output routing,
- event mutation,
- serialization,
- benchmarks.

## Acceptance

```text
Debug build: PASS
Release build: PASS
Tests: PASS
Warnings: 0 where practical
```

The first task must establish a clean foundation only.

---

# 61. Specification Freeze

Sections defining the following concepts are considered architecture constraints for the initial implementation:

- explicit configuration,
- explicit state,
- explicit input event,
- bounded outputs,
- no hidden time,
- no hidden randomness,
- no external side effects,
- no heap allocation in primitive execution,
- deterministic execution,
- C implementation,
- fixed-width integer arithmetic,
- high-level behaviors implemented by composition.

Any change to these constraints requires explicit architectural review.

---

# 62. Summary

The system being designed is a deterministic programmable communication fabric based on a small mathematical execution cell.

The intended analogy is not:

```text
new protocol
```

but rather:

```text
FPGA-like programmable fabric
```

where:

- the primitive is the programmable cell,
- state is local cell memory,
- configuration is the cell program,
- graph edges are interconnect,
- event tokens are signals/data,
- the graph is the configured communication behavior,
- a future compiler maps higher-level behavior into primitive graphs.

The immediate goal is not to prove that the architecture is correct.

The immediate goal is to build the smallest correct implementation capable of experimentally answering that question.
