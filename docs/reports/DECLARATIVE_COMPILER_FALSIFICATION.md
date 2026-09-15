# Declarative compiler falsification result

Result: the current primitive/Program IR cannot yet support the requested small
general compositional compiler without semantic cheating.

The runtime envelope now owns and deterministically propagates bounded scratch,
but `pnp_config_t` has no scratch source or scratch update destination. Therefore
pass-through, conditional drop, rewrite, and persistent counter lower normally,
while sequence-correlated ACK/retry compositions cannot move an intermediate
event-local value between generic stages. Treating a state-domain word as that
temporary value would incorrectly persist it across unrelated FIFO events and
fan-out branches. Generating RETRY-specific cells would merely retain the old
special-case compiler.

The missing capability is a validated primitive-layer scratch read/write
operation (including explicit branch-copy semantics), or an equally generic
Program IR operation lowered to the envelope. Until that exists, unsupported
generic scratch expressions must be rejected before runtime. The existing
single-slot RETRY helper remains explicit and sequence-correlated; no claim of a
general reliability language is made.
