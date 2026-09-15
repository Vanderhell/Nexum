# Nexum v1.1.0 release gate

Status: **PASS — v1.1.0 is ready to tag after manual main push**

Audited release-candidate commit before this documentation audit: `db55f54dad2e1091a279056441bf8108982f52f2`.

## Complete supported matrix

| Platform/toolchain | Configuration | Tests | Result |
|---|---|---:|---|
| Windows MSVC 19.42 | Debug | 18/18 | PASS |
| Windows MSVC 19.42 | Release | 18/18 | PASS |
| Linux GCC | Debug | 18/18 | PASS |
| Linux GCC | Release | 18/18 | PASS |
| Linux Clang | Debug | 18/18 | PASS |
| Linux Clang | Release | 18/18 | PASS |
| Linux Clang ASan | Debug | 18/18 | PASS, no findings |
| Linux Clang UBSan | Debug | 18/18 | PASS, no findings |

Linux results were run manually in WSL using clean, separate CMake build trees and the documented sanitizer flags. Windows Clang is not a declared supported release configuration.

## Correctness, performance, and memory

All 144 configuration/test executions passed. The planner now uses checked
reverse-topological multiplicity propagation. Its new runtime-backed regression
suite covers reconvergent and overflow cases. Frozen primitive behavior is
unchanged. Route lowering remains
bounded O(R²), graph emissions traverse only their indexed `(node, output port)`
routes, and finalized Program IR injection uses deterministic binary lookup.

The routing index is derived once per `pnp_graph_run()` before event processing and stored in bounded automatic storage. It is absent from `pnp_graph_t`, whose original definition-only layout is enforced by a compile-time regression assertion. Runtime and lowering remain heap-free. Scratch remains four 64-bit words per queue envelope and fan-out copies it by value.

## Semantic and compatibility impact

Graph failures have documented deterministic partial-commit semantics. Logical outputs consistently use IDs 0..255. RETRY ACKs must match the pending sequence and terminal failure closes the pending slot exactly once.

`pnp_graph_t` has no ABI/layout growth from routing preparation and existing `pnp_graph_run()` callers require no API change. `pnp_graph_event_t` intentionally includes the bounded scratch envelope introduced during v1.1 hardening; this changes layout for callers that store queue records, but the project labels these preliminary structures as not yet stable ABI. New finalization, planning, and scratch access functions are additive.

## Remaining issues

- P0: none.
- P1: none.
- P2: a generic declarative compiler still needs primitive-level scratch read/write operations. The explicit sequence-correlated single-slot RETRY helper is supported; no general reliability-language claim is made.

Project metadata is set to `1.1.0`, release notes are prepared, and the supported
matrix passes. No Git tag or GitHub release was created.
