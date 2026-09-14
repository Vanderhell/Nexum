# Programmable Network Primitive
# Implementation Prompt Pack for Codex

**Purpose:** Sequential implementation prompts only  
**Language:** C11  
**Authoritative architecture:** `SPEC.md`  
**Execution model:** one prompt at a time, in order  
**Target:** deterministic, bounded, side-effect-free programmable primitive and deterministic graph runtime

---

# How to Use This Pack

Run the prompts in order.

Do **not** give Codex the entire pack as one implementation request.

For each step:

1. provide the repository,
2. ensure `SPEC.md` is present,
3. give Codex exactly one prompt,
4. require it to inspect the current implementation first,
5. require build/tests before completion,
6. review the result,
7. continue only after the current prompt is clean.

`SPEC.md` is the architectural source of truth.

If this prompt pack conflicts with `SPEC.md`, `SPEC.md` wins.

The agent must not reinterpret the architecture for convenience.

---

# Global Rules for Every Prompt

These rules apply to **every prompt in this pack**, even when not repeated.

```text
Read SPEC.md completely before changing code.

Inspect the current repository and existing implementation before editing.

Implement only the scope explicitly requested by the current prompt.

Do not redesign the architecture.

Do not add high-level behavior opcodes such as:
ROUTE, FILTER, RETRY, ACK, BROADCAST, MULTICAST, DEDUP,
TIMEOUT, FAILOVER, LOAD_BALANCE, RATE_LIMIT.

Do not add sockets, networking APIs, threads, locks, callbacks,
function-pointer extensibility, persistence, JIT, scripting,
plugins, or OS-specific runtime dependencies.

Primitive execution must remain:
- deterministic,
- bounded,
- free of external side effects,
- free of heap allocation,
- free of hidden time,
- free of hidden randomness,
- free of mutable global state.

Use C11.

Do not introduce C++.

Do not add third-party runtime dependencies unless explicitly requested.

Do not create commits unless explicitly requested.

Do not push.

Do not rewrite Git history.

Do not add yourself as Author or Co-Author.

Do not weaken tests to make them pass.

Do not suppress real warnings without justification.

Do not perform unrelated refactoring.

If the requested feature cannot be implemented without changing the
mathematical contract, STOP implementation of that extension and report:
1. the exact blocker,
2. why the current model is insufficient,
3. the smallest possible architectural extension,
4. its impact on determinism,
5. its impact on bounded execution,
6. its impact on state/configuration size.

At the end report exactly:

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

---

# PROMPT 00 — Repository Audit and Baseline

```text
Read SPEC.md completely.

Perform a read-only audit of the repository before implementing anything.

Objectives:
1. Identify the current repository structure.
2. Identify build system state.
3. Identify all existing public headers.
4. Identify all source files.
5. Identify all tests.
6. Identify whether any implementation already violates SPEC.md.
7. Identify whether any dynamic allocation, global mutable state,
   hidden time/randomness, OS I/O, callbacks, function pointers,
   threads, locks, or high-level behavior opcodes already exist.
8. Identify compiler/toolchain assumptions.
9. Build the current project if possible.
10. Run all existing tests if possible.

Do not modify files.

Produce a precise baseline report.

If the repository is empty or only contains SPEC.md, state that clearly.

Do not propose architecture changes unless an existing conflict with SPEC.md
must be highlighted.

Return the standard report format, but Changed files must be:
- none
```

---

# PROMPT 01 — Project Skeleton

```text
Read SPEC.md completely and inspect the current repository.

Create the minimal C11 project skeleton described by SPEC.md.

Required structure:

/
├─ CMakeLists.txt
├─ README.md
├─ SPEC.md
├─ include/
│  ├─ pnp_types.h
│  ├─ pnp_result.h
│  ├─ pnp_primitive.h
│  └─ pnp_graph.h
├─ src/
│  ├─ pnp_internal.h
│  ├─ pnp_primitive.c
│  ├─ pnp_validate.c
│  └─ pnp_graph.c
└─ tests/
   └─ test_smoke.c

Requirements:
- C11.
- No C++.
- No third-party runtime dependencies.
- Build a static library for the core.
- Build a smoke-test executable.
- Debug and Release configurations must compile.
- Public headers must contain only preliminary public API contracts.
- Internal details stay in src/pnp_internal.h.
- No graph runtime behavior yet.
- No ALU behavior yet.
- No predicate behavior yet.
- No routing behavior yet.
- No dynamic allocation in primitive execution.
- Do not invent final binary serialization formats.

Implement only enough stub behavior for the build and smoke test to pass.

The README should be minimal:
- project purpose in a few lines,
- build commands,
- test command,
- point to SPEC.md.

Do not duplicate SPEC.md in README.

Acceptance:
- Debug build PASS.
- Release build PASS.
- smoke test PASS.
- no unnecessary files.
```

---

# PROMPT 02 — Core Public Types

```text
Read SPEC.md completely and inspect existing code.

Implement the initial core public data types only.

Required concepts:
- pnp_word_t based on uint64_t,
- result/status enum,
- fixed-size event structure,
- fixed-size state representation,
- primitive configuration structure,
- output item,
- output buffer.

Requirements:
- use stdint.h fixed-width types,
- no signed arithmetic in the primitive model,
- no pointers inside serialized mathematical state,
- no mutable external pointer inside primitive configuration,
- no callbacks,
- no function pointers,
- no OS handles,
- no heap-owned fields.

Event must provide preliminary fields for:
- type,
- input_port,
- sequence,
- epoch,
- flags,
- payload_ref,
- payload_length,
- fixed metadata word array.

State must use a fixed word array.

Output buffer must have fixed capacity.

Important:
- choose conservative compile-time constants for v0.1,
- define them in one obvious place,
- do not claim these values are permanent ABI,
- document them as preliminary implementation limits.

Add compile-time assertions where portable C11 _Static_assert can verify
important assumptions.

Add tests for:
- sizeof fundamental types where meaningful,
- zero initialization,
- maximum field values,
- output capacity constant,
- state word count constant.

Do not implement ALU execution yet.
```

---

# PROMPT 03 — Configuration Validation Foundation

```text
Read SPEC.md completely and inspect current code.

Implement the configuration validation foundation.

Goal:
Invalid primitive configuration should be rejected before execution.

Implement:
- pnp_validate_config(...)
- validation helpers kept internal,
- validation of all currently existing enum ranges,
- validation of all state indexes,
- validation of all event-field indexes,
- validation of shift/bit indexes if those fields already exist,
- validation of output masks against PNP_MAX_OUTPUTS,
- validation of reserved fields if applicable.

Requirements:
- validation must be deterministic,
- no allocation,
- no logging,
- no mutation of config,
- no hidden global state,
- fail with explicit pnp_result_t values.

Do not add missing execution behavior just to validate it.

Add table-driven tests covering:
- valid minimum configuration,
- valid maximum boundary values,
- each invalid enum,
- each invalid index,
- invalid output mask,
- invalid reserved value where relevant.

Acceptance:
Every invalid field has at least one explicit test.
```

---

# PROMPT 04 — Operand Source Model

```text
Read SPEC.md completely and inspect current code.

Implement operand source selection for the primitive.

Required source kinds:
- ZERO
- ONE
- CONSTANT
- EVENT_FIELD
- STATE_WORD
- SEQUENCE
- EPOCH
- FLAGS
- INPUT_PORT

Implement a compact source descriptor.

Implement an internal function equivalent to:

pnp_word_t pnp_read_source(
    const pnp_source_t *source,
    const pnp_state_t *state,
    const pnp_event_t *event
);

Requirements:
- deterministic,
- no allocation,
- no mutation,
- validated configuration must guarantee index safety,
- checked tests must also verify invalid descriptors are rejected by validator,
- do not silently clamp indexes,
- do not reinterpret signed values.

Add exhaustive tests for every source kind and boundary index.

Do not implement ALU operations yet beyond what is necessary to test source reading.
```

---

# PROMPT 05 — Minimal ALU

```text
Read SPEC.md completely and inspect current code.

Implement the initial word ALU.

Required opcodes only:
- PASS_A
- ADD
- SUB
- AND
- OR
- XOR
- SHL
- SHR

Semantics:
- pnp_word_t is uint64_t.
- ADD and SUB use defined unsigned modulo-2^64 behavior.
- SHL and SHR are logical unsigned shifts.
- shift counts >= 64 must never reach unchecked execution;
  validator must reject invalid constant/configured shift semantics.
- If shift count comes from runtime input, define one deterministic rule
  consistent with SPEC.md and document it explicitly.
  Prefer masking or explicit rejection only if the model can remain bounded
  and deterministic. Do not introduce undefined behavior.

Implement internal ALU evaluation.

Do not add:
- MUL,
- rotate,
- popcount,
- min/max,
- saturating arithmetic,
- behavior-specific operations.

Add unit tests:
- all opcodes,
- 0,
- UINT64_MAX,
- overflow,
- underflow,
- bit patterns,
- shift 0,
- shift 1,
- shift 63,
- runtime shift edge behavior.

Add determinism repetition tests for ALU-only primitive execution.

No graph runtime changes.
```

---

# PROMPT 06 — Predicate Engine

```text
Read SPEC.md completely and inspect current code.

Implement the minimal predicate engine.

Required predicates:
- ALWAYS
- EQ
- LT_U
- BIT_SET

Add one inversion flag.

The predicate may evaluate the ALU result against the configured comparison
source/value as defined by the current data model.

Requirements:
- unsigned semantics only,
- BIT_SET must validate bit index,
- no redundant predicate opcodes such as NE, GT, GE unless derivable through
  operand order and inversion,
- no behavior-specific predicates.

Add tests proving:
- ALWAYS true,
- ALWAYS + invert false,
- EQ equal/not equal,
- LT_U boundaries,
- BIT_SET for bit 0,
- BIT_SET for bit 63,
- inverted predicates,
- UINT64_MAX boundaries.

Do not add state updates or graph behavior beyond existing stubs.
```

---

# PROMPT 07 — Primitive Execution Core v1

```text
Read SPEC.md completely and inspect current code.

Implement the first complete deterministic primitive execution path for:

COMPUTE -> DECIDE

using:
- operand source A,
- operand source B,
- ALU opcode,
- predicate.

The execution function must:
1. reject null required arguments,
2. copy or initialize output structures deterministically,
3. read operands,
4. compute R,
5. evaluate predicate G,
6. return success.

At this stage:
- do not mutate state,
- do not emit graph outputs,
- do not mutate event fields.

Expose only the public result necessary for later stages.
If R/G need to remain internal, keep them internal.

Requirements:
- no heap allocation,
- no globals,
- no logging,
- no system calls,
- no undefined behavior,
- input state/event/config must not be modified.

Add tests that execute the same configuration/state/event at least 10,000 times
from identical inputs and verify bit-identical results.

Do not optimize yet.
```

---

# PROMPT 08 — Conditional State Update

```text
Read SPEC.md completely and inspect current code.

Implement conditional state update.

Requirements:
- zero or one state-word write per primitive execution,
- state destination is configured,
- update happens only according to predicate result and configured update mode,
- untouched state words remain unchanged,
- state_in must never be modified,
- state_out must always be fully initialized deterministically.

Initial update source options:
- NONE
- RESULT
- OPERAND_A
- OPERAND_B
- CONSTANT
- EVENT_FIELD

Do not add multiple writes.

Do not add arbitrary expressions to state update.

Validator must reject:
- invalid state destination,
- invalid event-field source,
- invalid update mode.

Add tests:
- no write,
- predicate true write,
- predicate false no-write,
- every allowed update source,
- first state word,
- last state word,
- unchanged-word byte comparison,
- repeated deterministic execution.
```

---

# PROMPT 09 — Output Emission Mask

```text
Read SPEC.md completely and inspect current code.

Implement bounded output emission.

Use:
- emit_true mask,
- emit_false mask.

The selected mask depends only on the predicate result.

For every set bit in the selected mask:
- produce exactly one output item,
- assign the corresponding logical output port,
- copy the input event deterministically.

Requirements:
- output count <= PNP_MAX_OUTPUTS always,
- zero mask emits zero outputs,
- output ordering must be deterministic:
  ascending output-port order,
- no allocation,
- no send/network calls,
- input event must not be modified,
- output buffer must be initialized deterministically.

Validator must reject masks using ports outside PNP_MAX_OUTPUTS.

Add tests:
- no output,
- one output,
- lowest port,
- highest port,
- multiple ports,
- all ports,
- predicate true mask,
- predicate false mask,
- deterministic output ordering.
```

---

# PROMPT 10 — Event Mutation

```text
Read SPEC.md completely and inspect current code.

Implement optional mutation of at most one metadata event field before emission.

Initial mutation source options:
- NONE
- RESULT
- OPERAND_A
- OPERAND_B
- STATE_WORD
- CONSTANT

Rules:
- original event input must never be modified,
- mutation is applied to the emitted event copy,
- all outputs from one primitive execution receive the same mutated event,
- no per-output mutation,
- no payload dereference,
- no arbitrary memory write,
- only the fixed metadata field array may be mutated in v0.1.

Validator must reject:
- invalid mutation destination,
- invalid state source index,
- invalid mutation mode.

Add tests:
- mutation disabled,
- result mutation,
- constant mutation,
- state mutation,
- first metadata field,
- last metadata field,
- fan-out receives identical mutated event copies,
- source event remains byte-identical.
```

---

# PROMPT 11 — Primitive Determinism and Integrity Test Suite

```text
Read SPEC.md completely and inspect current code.

Do not add new primitive features.

Build a dedicated deterministic integrity test suite.

Test the full primitive:

P_config(state,event) -> (state_out, outputs)

Required deterministic tests:
- at least 10,000 repeated executions per representative vector,
- identical input/config/state each iteration,
- byte-identical state_out,
- identical status,
- identical output count,
- identical output ports,
- byte-identical emitted events.

Cover combinations:
- stateless pass,
- stateless drop,
- ALU overflow,
- predicate false,
- state update,
- event mutation,
- maximum fan-out,
- max field values,
- zero values.

Also verify input immutability:
- config unchanged,
- state_in unchanged,
- event_in unchanged.

If structure padding prevents safe byte comparison, fix test methodology or
data initialization explicitly. Do not rely on uninitialized padding.

No architecture changes in this prompt.
```

---

# PROMPT 12 — Fast-Path Validation Boundary

```text
Read SPEC.md completely and inspect current code.

Review the current execution/validation boundary.

Implement a clean two-stage model only if it improves clarity without changing
public semantics:

1. configuration validation,
2. execution of already-valid configuration.

The public API may remain checked.

An internal validated fast path may be added.

Requirements:
- checked public execution must never invoke undefined behavior on invalid config,
- validated fast path may assume validation guarantees but must remain bounded,
- do not duplicate execution logic unnecessarily,
- do not add function-pointer dispatch,
- do not add JIT,
- do not add dynamic code generation.

Add tests proving invalid configuration is rejected before dangerous indexing
or shifts.

Benchmarking is not required yet.

If the current code is already clean, do not refactor merely to satisfy this prompt.
Report that no structural change was necessary.
```

---

# PROMPT 13 — Graph Data Structures

```text
Read SPEC.md completely and inspect current code.

Implement graph data structures only.

Required concepts:
- node ID,
- primitive instance,
- per-node configuration,
- per-node state,
- directed edge from source node/output port to destination node/input port,
- fixed-capacity graph event queue,
- graph event envelope:
  node_id,
  input_port,
  event.

Requirements:
- graph execution storage must be bounded,
- no heap allocation in graph execution,
- graph construction may use caller-provided fixed-capacity storage,
- prefer explicit capacities,
- no threads,
- no locks,
- no callbacks,
- no network I/O.

Implement graph validation for:
- valid node IDs,
- valid output ports,
- valid destination input ports,
- edge capacity,
- node capacity,
- queue capacity,
- duplicate edge semantics must be explicit and deterministic.

Do not execute graph events yet except minimal smoke tests for construction/validation.
```

---

# PROMPT 14 — Fixed-Capacity Ring Queue

```text
Read SPEC.md completely and inspect current code.

Implement the deterministic fixed-capacity graph event ring queue.

Requirements:
- caller/runtime-owned preallocated storage,
- push,
- pop,
- empty check,
- full check,
- count,
- deterministic FIFO ordering,
- explicit overflow result,
- no silent drop,
- no allocation,
- no locking,
- no atomics,
- no threads.

Handle wrap-around correctly.

Add focused tests:
- empty queue,
- one push/pop,
- fill exactly to capacity,
- reject one extra push,
- drain,
- wrap-around,
- repeated wrap-around,
- FIFO order after wrap,
- capacity 1 if supported,
- maximum configured test capacity.

Do not add graph scheduling yet.
```

---

# PROMPT 15 — Deterministic Single-Thread Graph Runtime

```text
Read SPEC.md completely and inspect current code.

Implement the first graph runtime.

Execution model:
- single-threaded,
- FIFO event queue,
- one graph event popped at a time,
- execute target primitive,
- update that node's state,
- translate primitive output ports through graph edges,
- enqueue resulting graph events in deterministic order.

Deterministic ordering rules:
1. FIFO input event order,
2. primitive output ports ascending,
3. for multiple edges from the same output port, use stable graph edge order.

Graph runtime must:
- reject invalid graph before running,
- never allocate in execution,
- detect queue full,
- return explicit error,
- preserve node state deterministically,
- never call external I/O.

Add tests:
- one-node pass,
- linear chain,
- branch,
- fan-out,
- two edges from one port,
- stable edge order,
- queue overflow,
- invalid node,
- invalid edge,
- multiple external input events.

Do not add parallelism.
```

---

# PROMPT 16 — Execution Budgets and Cycles

```text
Read SPEC.md completely and inspect current code.

Add bounded execution budgets to the graph runtime.

Required limits:
- max_steps,
- max_emitted_events.

Each primitive execution consumes one step.

Each graph event generated from primitive output contributes to emitted-event
accounting according to one explicitly documented rule.

Requirements:
- cycles are structurally allowed,
- infinite feedback must terminate with explicit limit error,
- no hidden watchdog thread,
- no wall-clock timeout,
- no sleep,
- no system clock.

Add tests:
- acyclic graph under budget,
- exact max_steps boundary,
- one step over boundary,
- finite cycle that terminates through state/predicate logic,
- infinite cycle stopped by max_steps,
- emitted-event budget exact boundary,
- emitted-event budget exceeded.

Do not attempt static boundedness proof yet.
```

---

# PROMPT 17 — External Input and Observable Output Model

```text
Read SPEC.md completely and inspect current code.

Define and implement a deterministic way to:
- inject an external event into a graph node/input port,
- collect graph outputs that leave the configured graph boundary.

Do not introduce network I/O.

Use fixed-capacity caller-owned buffers.

Requirements:
- explicit input target node/input port,
- explicit logical graph output representation,
- deterministic output order,
- bounded output collection,
- explicit output-buffer-full error,
- no allocation.

Add tests:
- one input -> one external output,
- one input -> multiple external outputs,
- zero external outputs,
- multiple injected inputs,
- output ordering,
- output capacity boundary.

Do not add sockets, files, callbacks, or asynchronous delivery.
```

---

# PROMPT 18 — Replay Determinism Suite

```text
Read SPEC.md completely and inspect current code.

Do not add new features.

Implement graph-level D2 replay determinism tests.

For each test scenario:
1. create graph/configuration,
2. initialize all node state,
3. inject a fixed ordered input sequence,
4. run to completion,
5. capture all observable outputs,
6. capture final node state,
7. reset graph to identical initial state,
8. replay identical input sequence,
9. assert byte-identical outputs,
10. assert byte-identical final state.

Test scenarios:
- linear graph,
- branch,
- fan-out,
- stateful counter,
- conditional route,
- metadata rewrite,
- finite feedback loop,
- explicit time-event sequence.

Repeat complete replay multiple times.

Ensure no test relies on wall-clock time or random APIs.
```

---

# PROMPT 19 — Expressivity Tier 1

```text
Read SPEC.md completely and inspect current code.

Do not add new primitive opcodes or high-level runtime features.

Implement Tier 1 expressivity as tests/examples using only the existing primitive
configuration and graph composition.

Prove configurations for:

1. pass-through,
2. drop,
3. conditional drop,
4. static route,
5. conditional route,
6. fan-out,
7. broadcast,
8. metadata rewrite,
9. counter,
10. conditional counter.

For each behavior:
- construct the smallest reasonable graph,
- document primitive count,
- document state words used,
- document expected input/output,
- add automated tests.

Critical rule:
If any behavior cannot be expressed cleanly, do not add a behavior-specific opcode.

Instead stop that behavior implementation and report:
- exact limitation,
- primitive count attempted,
- why composition fails,
- smallest mathematical capability that would solve it.

Do not modify the architecture unless explicitly instructed after review.
```

---

# PROMPT 20 — Expressivity Tier 2

```text
Read SPEC.md completely and inspect current code.

Do not add high-level opcodes.

Attempt Tier 2 behaviors using only current primitives and graph composition:

1. sequence checking,
2. duplicate suppression for immediate duplicate sequence values,
3. threshold detection,
4. event count per explicit epoch,
5. toggle behavior,
6. finite-state machine,
7. priority classification,
8. simple rate counter driven by explicit time/epoch metadata.

For each behavior:
- implement as tests/examples,
- report primitive count,
- report state word count,
- report graph depth,
- verify deterministic replay.

Do not implement a general dedup window yet.

If the current model is insufficient for a behavior:
- do not patch around it with special-case code,
- document the exact missing expressive capability.

This prompt is primarily an architecture falsification test.
```

---

# PROMPT 21 — Explicit Time Events

```text
Read SPEC.md completely and inspect current code.

Implement only the conventions required for explicit logical time events.

Time must remain data.

Do not call any system clock.

Define a simple event convention for time:
- event type identifies TIME,
- metadata field contains logical timestamp or tick value.

Do not create a timer service.

Add examples/tests showing primitives can:
- store last time token value,
- compare a later explicit time value,
- detect logical deadline threshold using unsigned arithmetic under documented assumptions.

All tests must inject explicit time values.

Replay must be identical.

No retry behavior yet.
```

---

# PROMPT 22 — Timeout Behavior by Composition

```text
Read SPEC.md completely and inspect current code.

Using only existing primitive operations, state, graph topology, and explicit TIME
events, build a timeout behavior as a test/example.

Do not add:
- TIMEOUT opcode,
- timer thread,
- clock access,
- scheduler timer,
- callback.

The graph should demonstrate:
1. receiving a data/control event,
2. storing sufficient bounded state,
3. receiving explicit TIME events,
4. detecting when a configured logical deadline has been reached,
5. emitting a timeout-indicating logical output token.

Requirements:
- deterministic,
- replayable,
- bounded,
- no external side effects.

Report:
- primitive count,
- state words,
- graph depth,
- limitations of unsigned timestamp arithmetic.
```

---

# PROMPT 23 — ACK Generation by Composition

```text
Read SPEC.md completely and inspect current code.

Build ACK generation purely as graph composition.

Do not add:
- ACK opcode,
- acknowledgement runtime subsystem,
- network send behavior.

Demonstrate:
- input event carrying sequence/correlation metadata,
- deterministic creation or mutation of an output event representing ACK,
- preservation of required correlation field,
- routing to a logical ACK output.

Add automated tests.

Report:
- primitive count,
- state requirement,
- event mutation requirements.

If ACK requires arbitrary event-type mutation not currently supported, do not
silently broaden event mutation. Report the exact gap first.
```

---

# PROMPT 24 — Retry State Machine by Composition

```text
Read SPEC.md completely and inspect current code.

Attempt a bounded retry state machine using only:
- explicit state,
- graph feedback,
- explicit TIME events,
- predicates,
- metadata mutation,
- output masks.

Do not add:
- RETRY opcode,
- timer service,
- sleep,
- network APIs.

Target behavior:
1. initial logical send event enters graph,
2. state records pending sequence and retry count,
3. ACK event clears pending state,
4. explicit TIME event after deadline causes another logical send output,
5. retry count increments,
6. retry stops at configured maximum,
7. final failure logical output is emitted.

Use a small fixed retry maximum in the test configuration.

Requirements:
- deterministic replay,
- bounded execution per injected event,
- fixed state,
- no allocation.

If current one-write-per-primitive semantics require multiple cells, use multiple
cells rather than extending the primitive.

Report primitive count and graph depth.
```

---

# PROMPT 25 — Failover Path Selection

```text
Read SPEC.md completely and inspect current code.

Build failover routing behavior by composition.

Inputs must explicitly carry link/path status as events or metadata.

Do not query real network interfaces.

Demonstrate:
- primary path selected when explicit state says available,
- secondary path selected when primary unavailable,
- deterministic transition when LINK_STATE-like event updates state.

Do not add FAILOVER opcode.

Add replay tests.

Report primitive count, state words, and routing depth.
```

---

# PROMPT 26 — Round-Robin Routing

```text
Read SPEC.md completely and inspect current code.

Build deterministic round-robin routing over a fixed number of logical outputs.

Do not add ROUND_ROBIN or LOAD_BALANCE opcode.

Use:
- state,
- ADD,
- predicate/composition,
- output masks.

Start with 2 outputs.

If clean, extend tests to 4 outputs without changing primitive semantics.

Requirements:
- deterministic sequence,
- state reset reproducible,
- no modulo opcode unless already mathematically justified by prior architecture review,
- if power-of-two masking is used, document it.

Report graph size and execution cost in primitive steps per input.
```

---

# PROMPT 27 — Finite-State Machine Demonstration

```text
Read SPEC.md completely and inspect current code.

Implement a nontrivial finite-state machine purely from primitive composition.

Use at least:
- 3 states,
- 3 event classes,
- one state-dependent transition,
- one transition that emits no output,
- one transition that emits output,
- one transition that mutates metadata.

Do not add FSM-specific runtime support.

Tests must prove:
- every transition,
- invalid/unmatched event behavior,
- deterministic final state,
- replay determinism.

Report primitive count and state words.
```

---

# PROMPT 28 — Expressivity Cost Audit

```text
Read SPEC.md completely and inspect current code and all expressivity tests.

Do not modify architecture.

Perform a quantitative audit of all implemented behaviors.

For each behavior collect:
- primitive count,
- graph edges,
- graph depth,
- state words,
- event mutations,
- max fan-out,
- primitive executions per input in the test case.

Behaviors should include all completed Tier 1/Tier 2 and timeout/ACK/retry/failover
demonstrations.

Produce a markdown report in:
docs/EXPRESSIVITY_COST.md

The report must identify:
- cheapest behaviors,
- most expensive behaviors,
- repeated primitive patterns,
- signs that the primitive is too weak,
- signs that an operation may deserve compiler fusion later,
- any architecture extension pressure.

Do not recommend adding a new opcode solely because a graph is longer.

No code optimization in this prompt.
```

---

# PROMPT 29 — Undefined Behavior and Portability Audit

```text
Read SPEC.md completely.

Perform a focused C correctness audit.

Inspect for:
- signed overflow,
- invalid shifts,
- out-of-bounds indexes,
- null dereference paths,
- uninitialized reads,
- padding-dependent comparisons,
- strict-aliasing violations,
- unaligned access assumptions,
- endian assumptions,
- narrowing conversions,
- invalid enum assumptions,
- integer wrap used unintentionally.

Fix concrete defects only.

Do not redesign architecture.

Add regression tests for every fixed defect.

Build with the strongest practical warnings available on the current compiler.

On GCC/Clang, where available, also run:
-fsanitize=address,undefined

Report sanitizer availability exactly.

Do not claim sanitizer PASS if runtime could not actually execute.
```

---

# PROMPT 30 — Warning Cleanliness and Build Matrix

```text
Read SPEC.md completely and inspect current build files.

Harden the build without changing runtime semantics.

Required:
- C11 explicitly required,
- Debug build,
- Release build,
- tests integrated with CTest,
- strong warnings.

For MSVC target approximately:
/W4

For GCC/Clang target approximately:
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wshadow

Do not blindly enable warnings that produce unavoidable platform noise.
Document any intentionally omitted warning.

Do not suppress project warnings globally merely to get a green build.

Run all tests in both Debug and Release where practical.

No architecture changes.
```

---

# PROMPT 31 — Property-Style Generated Determinism Tests

```text
Read SPEC.md completely and inspect current tests.

Add deterministic generated test coverage without introducing an external
property-testing dependency.

Use a small deterministic local test PRNG only inside test code with an explicit
fixed seed.

The production core must remain free of RNG.

Generate many valid combinations of:
- source descriptors,
- ALU ops,
- predicates,
- states,
- events,
- output masks,
- mutation settings.

For each generated valid case:
- execute multiple times from identical input,
- verify identical result,
- verify output count bound,
- verify no input mutation,
- verify only declared state word can differ,
- verify only declared metadata field can differ.

Because the test PRNG seed is fixed, failures must be reproducible.

Do not expose this PRNG in the public library.
```

---

# PROMPT 32 — Primitive Microbenchmark Harness

```text
Read SPEC.md completely and inspect current implementation.

Create a simple benchmark executable under benchmarks/ or tools/benchmarks/.

Do not add a third-party benchmark framework.

Measure at least:
- PASS_A / no state / one output,
- ADD,
- XOR,
- predicate true,
- predicate false,
- state update,
- event mutation,
- maximum fan-out.

Report:
- iterations,
- elapsed time,
- approximate ns/execution,
- executions/second.

Important:
- benchmark code may use a clock because it is outside the mathematical runtime,
- production primitive execution must not access the clock,
- prevent the compiler from optimizing the benchmark away,
- do not claim cycle-accurate results unless actually measured correctly.

Do not optimize production code in this prompt.
Only establish baseline measurements.
```

---

# PROMPT 33 — Graph Benchmark Harness

```text
Read SPEC.md completely and inspect current runtime.

Extend benchmark tooling for graph execution.

Benchmark:
- one node,
- chain of 4,
- chain of 16,
- one branch,
- max fan-out,
- small finite feedback graph.

Measure:
- events/second,
- primitive executions/second,
- approximate ns/input event,
- queue high-water mark if easy to expose internally for benchmark diagnostics.

Do not modify public API solely for benchmark convenience.
Do not optimize yet.

Record benchmark environment information:
- compiler,
- build type,
- architecture,
- OS,
- relevant compile flags.
```

---

# PROMPT 34 — Performance Profile and Hotspot Audit

```text
Read SPEC.md completely.

Use the existing benchmark harness and available profiler/compiler diagnostics
to identify actual hot paths.

Do not optimize based on intuition.

Inspect:
- source selection,
- opcode dispatch,
- predicate dispatch,
- event copy cost,
- output expansion,
- graph queue operations,
- edge traversal,
- state copying.

Produce:
docs/PERFORMANCE_BASELINE.md

Include:
- measured baseline,
- identified hotspots,
- likely optimization candidates,
- changes that would risk semantics,
- changes that are purely implementation-level.

Do not perform major optimization in this prompt.
```

---

# PROMPT 35 — Safe Low-Risk Optimization Pass

```text
Read SPEC.md completely.
Read docs/PERFORMANCE_BASELINE.md if present.

Perform only low-risk implementation optimizations supported by measurements.

Allowed examples:
- remove redundant copies,
- simplify validated index handling,
- reduce repeated initialization,
- improve local data layout,
- inline very small internal helpers when measured,
- reduce obviously redundant branches.

Forbidden in this prompt:
- semantic changes,
- new opcodes,
- JIT,
- function-pointer dispatch,
- threads,
- SIMD-specific public API,
- graph compiler,
- primitive fusion.

For every optimization:
1. keep old tests passing,
2. run determinism/replay tests,
3. rerun benchmarks,
4. report before/after numbers.

Revert any optimization that materially complicates code without measurable gain.
```

---

# PROMPT 36 — Static Graph Analysis Foundation

```text
Read SPEC.md completely and inspect graph structures.

Add a read-only graph analysis API or internal analysis module.

Compute deterministically:
- node count,
- edge count,
- total state bytes,
- configuration bytes if meaningful,
- declared queue capacity,
- maximum configured fan-out,
- graph maximum out-degree,
- whether cycles exist.

Do not attempt full boundedness proof yet.

Cycle detection must be deterministic.

No graph mutation.

Add tests for:
- empty graph if supported,
- chain,
- branch,
- DAG,
- self-cycle,
- multi-node cycle.

Do not claim D3 or boundedness proof.
```

---

# PROMPT 37 — D3 Eligibility Analysis: Conservative First Pass

```text
Read SPEC.md completely.

Implement only a conservative first-pass analysis related to D3
(schedule-independent determinism).

Do not claim general D3 proof.

The analyzer may classify only clearly safe/simple structures, for example:
- a purely linear chain,
- a DAG where no node receives from competing paths and no shared mutable state exists.

If a graph is not trivially provable, return:
NOT_PROVEN

Never return FAIL when the correct meaning is simply "not proven".

Never return PASS based on execution timing assumptions.

Add tests proving the analyzer is conservative.

Document exactly what subset is recognized.

No parallel execution.
```

---

# PROMPT 38 — Serialization Design Audit Only

```text
Read SPEC.md completely and inspect stabilized in-memory structures.

Do not implement serialization yet.

Produce:
docs/SERIALIZATION_DESIGN.md

Define requirements for future stable serialization of:
- primitive configuration,
- primitive state,
- graph topology,
- graph runtime state.

Address:
- versioning,
- endian encoding,
- integer widths,
- reserved fields,
- validation,
- corrupt input,
- forward compatibility,
- canonical representation,
- structure padding,
- pointer prohibition.

Do not serialize raw C structs directly.

Do not freeze a binary ABI in code.

This is a design checkpoint only.
```

---

# PROMPT 39 — Internal Serialization v0 for Tests

```text
Read SPEC.md completely.
Read docs/SERIALIZATION_DESIGN.md.

Implement a clearly marked INTERNAL/EXPERIMENTAL serialization format only if
the design document supports it.

Scope:
- primitive configuration,
- primitive state.

Do not serialize graph runtime yet.

Requirements:
- explicit byte order,
- explicit version,
- no raw struct dumps,
- bounds checking,
- corrupted data rejection,
- deterministic canonical output,
- no allocation in decode if caller supplies destination object.

Add round-trip tests and malformed-input tests.

Public API/documentation must state this format is not yet stable.
```

---

# PROMPT 40 — Fuzz Target Foundation

```text
Read SPEC.md completely and inspect current validators/parsers.

Add fuzz targets where the toolchain supports them.

Priority:
1. config validation,
2. experimental config/state decoder if present,
3. primitive execution using only already-validated generated configurations.

Use libFuzzer when available with Clang, otherwise keep fuzz target source
portable enough to integrate later.

Requirements:
- no false assumptions about input validity,
- no crashes,
- no OOB,
- no UB,
- no uncontrolled huge allocations,
- bounded test execution.

Do not make fuzz tooling a mandatory production dependency.

If fuzz runtime is unavailable, build the target where possible and report:
NOT RUN.
```

---

# PROMPT 41 — Full Architecture Conformance Audit

```text
Read SPEC.md completely.

Perform a full conformance audit of the implementation.

Check every architecture constraint:

- C11,
- deterministic primitive,
- explicit configuration,
- explicit state,
- explicit event input,
- bounded outputs,
- no hidden time,
- no hidden randomness,
- no external side effects in primitive execution,
- no heap allocation in primitive execution,
- no global mutable execution state,
- no behavior-specific opcodes,
- bounded graph queue,
- deterministic graph scheduling,
- explicit cycle budgets,
- replay determinism,
- payload not dereferenced by primitive core,
- no C++,
- no callbacks/function-pointer extensibility in core.

Search the repository, do not rely on assumptions.

Fix clear accidental violations.

Do not change architecture to make audit items easier.

Produce:
docs/CONFORMANCE_AUDIT.md

Classify every item:
PASS
FAIL
NOT IMPLEMENTED
NOT APPLICABLE

Every FAIL must include file/function evidence.
```

---

# PROMPT 42 — v0.1 Release Gate

```text
Read SPEC.md completely.
Read all existing audit documents.

Do not add new features.

Run the full release gate.

Required checks:
1. clean Debug build,
2. clean Release build,
3. all unit tests,
4. determinism tests,
5. replay tests,
6. Tier 1 expressivity tests,
7. completed Tier 2 tests,
8. timeout/ACK/retry/failover tests if implemented,
9. graph cycle budget tests,
10. generated property-style tests,
11. sanitizers where supported,
12. build warnings review,
13. conformance audit review.

Produce:
docs/V0_1_RELEASE_GATE.md

Use only:
PASS
FAIL
NOT RUN
NOT APPLICABLE

Do not call v0.1 ready if a mandatory SPEC.md requirement is FAIL.

Do not hide incomplete sanitizer/fuzzer verification.

Do not implement fixes in this prompt unless they are trivial release-blocking
defects with clear tests. For substantial defects, report them instead.
```

---

# PROMPT 43 — Architecture Falsification Review

```text
Read SPEC.md.
Read:
- docs/EXPRESSIVITY_COST.md
- docs/PERFORMANCE_BASELINE.md
- docs/CONFORMANCE_AUDIT.md
- docs/V0_1_RELEASE_GATE.md
when present.

Do not modify code.

Evaluate whether the current mathematical primitive should survive unchanged.

Analyze evidence only.

Questions:
1. Which behaviors are awkward to express?
2. Which behaviors require too many primitive cells?
3. Which repeated cell sequences appear everywhere?
4. Is one ALU operation per cell still defensible?
5. Is one predicate sufficient?
6. Is one state write sufficient?
7. Is one event-field mutation sufficient?
8. Is output-mask emission sufficient?
9. Is event copying too expensive?
10. Is fixed state size causing waste?
11. Are graph cycles manageable?
12. Is determinism still clean?
13. Did any hidden runtime complexity emerge?
14. Does the design still map naturally to MCU/CPU/FPGA-like execution?

Produce:
docs/ARCHITECTURE_FALSIFICATION.md

For every proposed architectural change include:
- evidence,
- benefit,
- cost,
- impact on determinism,
- impact on bounded execution,
- impact on state/config size,
- whether the same benefit could instead be achieved by compiler optimization.

Do not change the architecture in this prompt.
```

---

# PROMPT 44 — Architecture Freeze Candidate

```text
Read SPEC.md and docs/ARCHITECTURE_FALSIFICATION.md.

Do not change code initially.

Prepare a concrete candidate for the next architecture freeze.

The result must separate:

KEEP:
- concepts proven useful.

CHANGE:
- concepts with evidence-backed problems.

DEFER:
- optimizations/features that are not required for the mathematical model.

REMOVE:
- concepts that added complexity without sufficient value.

Produce:
docs/ARCHITECTURE_FREEZE_CANDIDATE.md

Do not make speculative additions.

Every CHANGE must trace back to test, benchmark, or expressivity evidence.

Do not automatically implement changes.
Implementation requires explicit user approval after this checkpoint.
```

---

# Optional Post-v0.1 Prompts

The following prompts must be used only after the primitive architecture is
reviewed and explicitly accepted.

---

# PROMPT 45 — Caller-Provided Variable Per-Node State

```text
Use only after explicit architecture approval.

Implement bounded per-node state sizing while preserving:
- explicit state,
- preallocation,
- deterministic execution,
- no heap allocation in execution.

Do not introduce pointers into serialized mathematical state.

Update graph memory accounting and tests.

Measure memory savings versus fixed-size node state.
```

---

# PROMPT 46 — Primitive Fusion Prototype

```text
Use only after explicit architecture approval.

Prototype compile-time/build-time fusion of adjacent compatible primitives.

Fusion must be an optimization only.

Semantics must remain equivalent to the unfused graph.

Requirements:
- retain a reference unfused execution path in tests,
- differential-test fused vs unfused,
- same outputs,
- same final state,
- same observable ordering.

Do not add behavior-specific fused opcodes to the public mathematical model.
```

---

# PROMPT 47 — SIMD Batch Prototype

```text
Use only after explicit architecture approval.

Prototype SIMD execution for batches of independent primitive instances/events.

Do not change scalar semantics.

Requirements:
- scalar path remains reference implementation,
- differential tests against scalar,
- no change to event ordering semantics,
- no dependency on SIMD in public API.

Report actual benchmark gain.
```

---

# PROMPT 48 — Parallel Graph Runtime Research Prototype

```text
Use only after explicit architecture approval.

Do not replace the deterministic single-thread runtime.

Create an experimental parallel runtime prototype only for graphs that the
analyzer can conservatively classify as safe.

Reference result must remain the single-thread runtime.

Requirements:
- identical observable output,
- identical final state,
- repeated stress tests,
- never claim general D3 support.

If deterministic equivalence cannot be proven/tested for the supported subset,
stop and report the limitation.
```

---

# Final Rule

The implementation is an experiment intended to **validate or falsify the
primitive model**.

The correct outcome is not necessarily "finish all features".

If evidence shows that the primitive is mathematically weak, inefficient, or
requires hidden complexity, preserve the evidence and stop before papering over
the problem with special cases.

The model must earn its complexity.
