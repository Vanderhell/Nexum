# Security model

This document describes implemented properties, not a guarantee of security.

## Implemented boundaries

Primitive and graph execution use no heap allocation. Queues, nodes, edges, state
domains, outputs, Program IR, and builder/compiler storage are caller-owned or
fixed-capacity. Configuration, graph, and IR validation reject invalid selectors,
indexes, ports, references, reserved values, and capacity relationships.

Arithmetic uses fixed-width unsigned types with defined modulo behaviour. Runtime
shifts are masked and invalid constant shifts are rejected. Primitive outputs are
bounded by `PNP_MAX_OUTPUTS`; graph queues and external buffers have explicit
capacities. Graph execution has caller-supplied step and emission budgets.

The primitive core has no hidden clock or randomness. It contains no networking,
filesystem I/O, callback dispatch, logging, threads, or synchronization APIs. Time,
randomness, and link status must enter as explicit event data.

Canonical v0 config/state decoding validates its input. The fuzz target exercises
raw configuration validation and config/state decoding. Cross-toolchain vectors
check deterministic results. The full suite has been run under Clang ASan and UBSan.

## Trust boundary

Callers remain responsible for valid object lifetimes, adequate storage, appropriate
execution budgets, synchronization around shared objects, and validation before
using any future unchecked path. Payload references are opaque values; the primitive
does not validate or dereference the referenced storage.

D1 and D2 are tested guarantees of the model. D3 is conservative and is not claimed
for all graphs. Resource exhaustion is reported as an error, not prevented by an
unbounded allocator.

Testing, fuzzing, sanitizers, and deterministic design do not prove the absence of
vulnerabilities or undefined behaviour. See [`../SECURITY.md`](../SECURITY.md) for
private reporting guidance.
