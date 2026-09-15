# v1.1.0 hardening baseline

The pre-hardening runtime is deterministic and heap-free, but has three known hot-path limitations: route lowering ranks every route against every other route for every output position (cubic), graph execution scans every edge for each primitive output, and `nexum_program_inject` validates and scans the full Program IR on every call.

Graph errors use observable partial-commit behavior: a popped source event is consumed; primitive state commits before routing; successful earlier routes and external outputs remain committed; the failing route is not committed; queued work remains queued. Step exhaustion occurs before consuming the next event. Emission exhaustion occurs before attempting its route.

Logical output IDs are unique and currently limited to 0..255 despite `NEXUM_PROGRAM_MAX_OUTPUTS` being 2048. This is a documented capacity mismatch to resolve in Prompt 6. Compiler RETRY is single-slot and presently accepts any ACK of the configured type without sequence correlation.
