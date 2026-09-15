# Nexum v1.1.0 release gate

Status: **FAIL — do not tag v1.1.0**

Audited implementation commit: `aa9fe6115c9a966069155bb5de6ec975773a862d`.

## Matrix

| Toolchain | Configuration | Result |
|---|---|---|
| MSVC 19.42 | Debug | 17/17 PASS |
| MSVC 19.42 | Release | 17/17 PASS |
| GCC 16.1 | Debug | 17/17 PASS |
| Clang 22.1 | Release | BLOCKED: local linker lacks `oldnames.lib` and `msvcrtd.lib` |
| ASan | — | not available in the completed Windows matrix |
| UBSan | — | not available in the completed Windows matrix |

## Audit result

Route lowering changed from cubic to bounded quadratic ordering. Runtime routing
uses a precomputed per-node/port linked index and does not scan unrelated edges
per emission. Finalized Program IR injection uses deterministic binary lookup
without whole-program validation. Runtime/lowering remain heap-free. Scratch is
four words per queue envelope and copied by value. Failure behavior is explicit
partial commit. RETRY ACKs are sequence-correlated and terminal failure closes
the pending slot. Logical output capacity is consistently 256.

Compatibility impact: `pnp_graph_t` and `pnp_graph_event_t` grew to hold routing
index and scratch storage. The project described these preliminary types as not
a stable ABI, but this remains a material source/binary-layout change requiring
review before a v1.1.0 tag.

## Open issues

- P1: the routing index is embedded in every `pnp_graph_t`; assess an adjacent
  caller-owned representation to reduce object size and compatibility impact.
- P1: complete Clang and sanitizer gates are unavailable in this environment.
- P2: generic declarative compilation remains blocked because primitives cannot
  read/write event-local scratch; the explicit RETRY helper is safe but is not a
  general composition language.
- P2: add dedicated planner and scratch regression executables beyond API-level
  coverage before claiming the new facilities fully hardened.

Project version metadata remains unchanged and no tag or release was created.
