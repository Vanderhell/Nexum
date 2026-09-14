#include <string.h>
#include "pnp_graph.h"

pnp_result_t pnp_graph_analyze(const pnp_graph_t *g, uint32_t queue_capacity, pnp_graph_analysis_t *a) {
    uint32_t i, j, removed = 0u, max_out = 0u, max_fan = 0u;
    if (g == NULL || a == NULL) return PNP_ERR_INVALID_ARGUMENT;
    if (pnp_graph_validate(g, queue_capacity) != PNP_OK) return PNP_ERR_INVALID_GRAPH;
    memset(a, 0, sizeof(*a)); a->node_count = g->node_count; a->edge_count = g->edge_count;
    a->state_bytes = (uint64_t)g->state_domain_count * (uint64_t)sizeof(pnp_state_domain_t);
    a->configuration_bytes = (uint64_t)g->node_count * (uint64_t)sizeof(pnp_config_t); a->queue_capacity = queue_capacity;
    for (i = 0u; i < g->node_count; ++i) {
        uint32_t degree = 0u;
        for (j = 0u; j < g->edge_count; ++j) if (g->edges[j].source_node == i) ++degree;
        if (degree > max_out) max_out = degree;
        for (j = 0u; j < PNP_MAX_OUTPUTS; ++j) {
            uint32_t fan = 0u, k; for (k = 0u; k < g->edge_count; ++k) if (g->edges[k].source_node == i && g->edges[k].source_port == j) ++fan;
            if (fan > max_fan) max_fan = fan;
        }
    }
    a->maximum_out_degree = max_out; a->maximum_fanout = max_fan;
    if (g->node_count != 0u) {
        uint8_t gone[PNP_GRAPH_MAX_NODES]; memset(gone, 0, sizeof(gone));
        for (;;) { uint32_t changed = 0u;
            for (i = 0u; i < g->node_count; ++i) if (!gone[i]) { int incoming = 0;
                for (j = 0u; j < g->edge_count; ++j) if (g->edges[j].destination_node == i && !gone[g->edges[j].source_node]) { incoming = 1; break; }
                if (!incoming) { gone[i] = 1u; ++removed; ++changed; }
            }
            if (changed == 0u) break;
        }
    }
    a->has_cycles = removed != g->node_count;
    if (!a->has_cycles) { int competing = 0, shared = 0; for (i = 0u; i < g->node_count; ++i) { uint32_t in = 0u; for (j = 0u; j < g->edge_count; ++j) if (g->edges[j].destination_node == i) ++in; if (in > 1u) competing = 1; for (j = i + 1u; j < g->node_count; ++j) if (g->nodes[i].state_domain == g->nodes[j].state_domain) shared = 1; } a->d3_status = (uint8_t)((competing || shared) ? PNP_D3_NOT_PROVEN : PNP_D3_PROVEN); }
    return PNP_OK;
}
