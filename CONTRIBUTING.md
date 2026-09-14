# Contributing to Nexum

Nexum explores a deterministic, bounded programmable communication core in C11.
Bug fixes, portability improvements, tests, documentation, analysis, and carefully
justified architecture proposals are welcome.

## Architecture freeze

The v0.1 primitive and graph semantics are frozen. Do not add or reinterpret an ALU
opcode, source selector, predicate, event destination, state-domain rule, FIFO rule,
or serialization rule in an ordinary feature pull request. A proposed freeze change
must be clearly labelled as an architecture proposal, explain why existing
composition is insufficient, assess compatibility and determinism, and include
falsification tests. Behaviour-specific opcodes require exceptional architectural
justification.

## Code and naming

Production code is portable C11. Existing runtime APIs retain the `pnp_*` prefix;
higher layers use `nexum_*`. Follow nearby formatting, use fixed-width types where
semantics depend on width, validate indexes and reserved fields, and keep public
headers self-contained.

Do not introduce hidden heap allocation, time, randomness, I/O, callbacks, or global
mutable execution state. Deterministic and bounded failure semantics must remain
explicit. Never weaken validation or bypass tests to make a change pass.

## Before opening a pull request

Build Debug and Release with an applicable supported compiler, run the complete
CTest suite, and keep all configured warnings clean. Bug fixes and features require
focused regression tests. Public API and architecture effects must be documented.
Sanitizer and fuzz runs are expected for memory-safety-sensitive changes.

A pull request should be narrowly scoped, explain motivation and observable effects,
list verification performed, and call out any compatibility, determinism,
boundedness, allocation, or security impact. Commits should be reviewable, use clear
imperative subjects, and must not include generated build products.

## Where to discuss work

* Bugs: GitHub Issues.
* Feature and architecture proposals: GitHub Issues using the feature template.
* Undisclosed vulnerabilities: private reporting described in `SECURITY.md`, never a public issue.
* General architecture questions: GitHub Discussions if maintainers enable it; otherwise use an Issue only when actionable.

Participation is governed by [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md).
