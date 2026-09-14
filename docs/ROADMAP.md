# Nexum roadmap

This is a research direction, not an API commitment or delivery schedule.

## Current

* Frozen deterministic primitive and graph runtime.
* Explicit state domains, bounded FIFO execution, and execution budgets.
* Validated Program IR and deterministic caller-owned lowering.
* Program/config builders and initial compiler helpers.
* Canonical v0 configuration/state serialization and hardened tests.

## Next

* Research event-local scratch architecture without weakening FIFO replay semantics.
* Revisit a general declarative compiler using retry as a falsification case.
* Resource-planner design for queues and complete program execution bounds.
* Program serialization design without prematurely freezing an unsafe ABI.
* Optimizer research with semantic-equivalence proofs and tests.

## Later

* Debugger and deterministic trace tooling.
* Graph visualization.
* ARM/MCU portability and resource validation.
* SIMD experiments.
* FPGA and other backend research.
