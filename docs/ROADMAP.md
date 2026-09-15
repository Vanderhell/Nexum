# Nexum roadmap

This is a research direction, not an API commitment or delivery schedule.

## Current

* Frozen deterministic primitive and graph runtime.
* Explicit state domains, bounded FIFO execution, and execution budgets.
* Validated Program IR and deterministic caller-owned lowering.
* Program/config builders and initial compiler helpers.
* Canonical v0 configuration/state serialization and hardened tests.
* Bounded event-local scratch propagation and deterministic resource planning.
* Indexed graph routing, finalized logical-input lookup, and sequence-correlated
  single-slot RETRY.

## Next

* Add generic primitive-level scratch read/write operations before revisiting a
  general declarative compiler; the current falsification test remains negative.
* Improve proof strength for resource planning where graph structure permits it.
* Program serialization design without prematurely freezing an unsafe ABI.
* Optimizer research with semantic-equivalence proofs and tests.

## Later

* Debugger and deterministic trace tooling.
* Graph visualization.
* ARM/MCU portability and resource validation.
* SIMD experiments.
* FPGA and other backend research.
