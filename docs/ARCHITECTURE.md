# Nexum architecture

Nexum separates logical program construction from the frozen execution core:

```text
Declarative frontend (future work)
              |
              v
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
      primitive execution
```

The declarative frontend is not implemented. Current compiler helpers are bounded
convenience compositions, not a text language or interpreter.

## Primitive and event

A primitive computes one fixed transition from an immutable `pnp_config_t`, one
`pnp_state_t`, and one `pnp_event_t`. Existing selectors feed a single ALU operation
and predicate. A successful predicate may update one state word, update one event
field, and select one bounded output mask. No new opcodes are introduced above this
layer.

Events have fixed-width TYPE, input port, sequence, epoch, flags, payload descriptor,
and metadata fields. The core does not dereference payload memory. Time and random
values must arrive as explicit events.

## State domains and graphs

A graph cell references a state domain. Sharing is explicit: cells that name the
same domain observe the same state; isolated domains do not alias. Directed edges
connect cell output ports to cell input ports or to external outputs.

The runtime processes a caller-owned FIFO. Edges are traversed in array order, so
ordering is semantic. Queue, external-output, step, and emitted-event limits are
explicit and failures are reported rather than clamped.

## Program IR and lowering

`nexum_program_t` is a bounded logical representation with embedded cells, domains,
edges, inputs, and outputs. It does not depend on runtime array addresses or queue
storage. Validation checks IDs, references, capacities, ports, routing order, and
every embedded primitive configuration.

Lowering sorts cells and domains by logical ID and routes by source, port, and
explicit route order. The caller provides node, edge, and state-domain arrays.
Lowering performs no heap allocation and reuses the frozen `pnp_graph_t` backend.
Finalization sorts the bounded logical-input table, allowing normal injection to
use binary lookup without revalidating the complete Program IR. Callers must not
mutate a finalized program without finalizing it again.

Before each graph run, the runtime derives a bounded routing index in automatic
storage. The index is not part of `pnp_graph_t` and preserves edge-array order.
Event processing follows only the indexed routes for the emitted node and port;
it does not scan unrelated edges.

## Graph failure contract

Graph execution uses deterministic partial commits. Once an event is popped it
is consumed, and its primitive state transition commits before any of its routes
are attempted. Routes commit in edge order. Queue entries and external outputs
created by earlier routes remain visible if a later route fails. The failing
route itself is not committed or included in `emitted_events`; pending queue
contents are retained. Step exhaustion is checked before popping the next event,
while emission exhaustion is checked before attempting a route. No rollback is
performed. Consequently callers may inspect or resume retained queued work, but
must treat the already committed state and outputs as final.

## Builder and compiler helpers

The Program Builder fills IR without manual array indexes, IDs, or route-order
accounting. It uses explicit capacities, latches the first error, and becomes
immutable after finalization. Configuration helpers construct ordinary validated
`pnp_config_t` values.

Compiler helpers currently cover pass, conditional drop/route, metadata or TYPE
rewrite, counter, ACK generation, and a single-slot retry composition. RETRY stores
one pending sequence, accepts only a sequence-correlated ACK, and produces one
terminal failure for the slot. These helpers are syntactic sugar over the builder
and frozen primitive semantics, not a general declarative compiler.

## Determinism

```text
D1  Same primitive configuration/state/event -> same result.
D2  Same ordered graph input stream -> same ordered output stream.
D3  Schedule independence; conservatively proven only for eligible graphs.
```

D1 and D2 are required properties. D3 is reported as `PROVEN` or `NOT_PROVEN`; the
latter is not automatically a runtime error.

## Bounded memory and execution

Production execution uses caller-owned fixed-capacity storage. Primitive fan-out is
bounded, graph queues are bounded, and graph runs require step and emission budgets.
The graph may contain cycles, but exhaustion of a budget terminates with an error.

## Event-local scratch

Each internal queue envelope owns four 64-bit scratch words and an explicit
initialized-word count. Injection zero-initializes it. Internal routing copies
the envelope by value, including at fan-out, so branches never alias mutable
scratch. Scratch ends with the envelope, is not exposed as external payload or
serialized, and replay remains byte-deterministic. Out-of-range access returns
an explicit capacity error.

`pnp_graph_event_t` grew to contain this envelope. Consumers providing queue
storage must rebuild against the v1.1 headers. The derived routing index does not
change the layout of `pnp_graph_t`.

## Resource planning

`nexum_program_plan()` deterministically reports structural counts, routing and
mapping entries, maximum per-port fan-out, scratch words, and storage size. Bound
statuses distinguish proven, conservative, and unproven results. The v1.1 pre-tag
audit found that the current acyclic emission calculation does not account for
reconvergent path multiplicity; its conservative status must not be relied upon
until that release blocker is corrected. Cycles are explicitly reported as
`NEXUM_BOUND_NOT_PROVEN`.

## Serialization

Canonical v0 serialization exists for `pnp_config_t` and `pnp_state_t` using an
explicit little-endian format. Stable Program IR or graph-program serialization is
not implemented and no hash ABI is defined.
