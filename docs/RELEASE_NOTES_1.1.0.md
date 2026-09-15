# Nexum 1.1.0 release notes

Nexum 1.1.0 hardens the experimental deterministic runtime without changing frozen primitive semantics.

Highlights:

* deterministic route lowering is now O(R²) rather than cubic;
* graph routing uses a bounded derived index and does not scan unrelated edges per emission;
* finalized Program IR input injection uses bounded binary lookup;
* graph failure behavior is explicitly defined as deterministic partial commit;
* logical output capacity consistently matches the 256-port runtime namespace;
* RETRY acknowledgements are sequence-correlated and terminal failure is single-shot;
* event-local scratch is bounded, queue-owned, and copied by value at fan-out;
* callers can inspect sound per-root-event DAG emission, step, and queue bounds,
  including reconvergent path multiplicity and explicit overflow/cycle status.

The supported MSVC, GCC, and Clang Debug/Release matrix passes, as do Linux Clang ASan and UBSan. Nexum remains experimental research software, not production-certified or formally verified.

Compatibility note: the derived routing index no longer changes `pnp_graph_t`. Queue records include the new bounded scratch envelope, so code allocating `pnp_graph_event_t` storage must be rebuilt against the v1.1 headers.
