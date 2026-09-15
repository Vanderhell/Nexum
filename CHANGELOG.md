# Changelog

Notable project changes are documented here in a Keep a Changelog-inspired format.
The project intends to follow Semantic Versioning after public releases begin.

## [Unreleased]

## [1.1.0] - 2026-09-15

### Added

* Deterministic bounded primitive and FIFO graph runtime with explicit state domains.
* Canonical v0 configuration and state serialization foundation.
* Nexum Program IR, independent validation, deterministic lowering, and requirements API.
* Zero-allocation Program Builder and primitive configuration helpers.
* Compiler foundation for basic behaviours and retry composition.
* Determinism, topology, expressivity, freeze, generated, serialization, retry,
  Program IR, builder, and compiler tests.
* Portable fuzz entry point, sanitizer support, and benchmark harness.

### Changed

* Reduced deterministic Program IR route lowering from cubic to quadratic time.
* Indexed graph routing by node and output port without changing `pnp_graph_t` layout.
* Added finalized binary lookup for logical program inputs.
* Defined deterministic partial-commit graph failure semantics.
* Aligned the logical output capacity with the representable 256-port namespace.
* Correlated compiler-generated RETRY acknowledgements by sequence and made terminal failure single-shot.
* Added bounded, value-copied event-local scratch envelopes and deterministic resource planning.
