#include <string.h>
#include "pnp_graph.h"
#include "pnp_internal.h"

void pnp_queue_init(pnp_queue_t *q, pnp_graph_event_t *storage, uint32_t capacity) {
    if (q != NULL) { q->items = storage; q->capacity = capacity; q->head = 0u; q->count = 0u; }
}
int pnp_queue_empty(const pnp_queue_t *q) { return q == NULL || q->count == 0u; }
int pnp_queue_full(const pnp_queue_t *q) { return q != NULL && q->capacity != 0u && q->count == q->capacity; }
uint32_t pnp_queue_count(const pnp_queue_t *q) { return q == NULL ? 0u : q->count; }
pnp_result_t pnp_queue_push(pnp_queue_t *q, const pnp_graph_event_t *e) {
    uint32_t tail;
    if (q == NULL || e == NULL || q->items == NULL || q->capacity == 0u || q->head >= q->capacity || q->count > q->capacity) return PNP_ERR_INVALID_ARGUMENT;
    if (q->count == q->capacity) return PNP_ERR_QUEUE_FULL;
    tail = q->head >= q->capacity - q->count ? q->head - (q->capacity - q->count) : q->head + q->count;
    q->items[tail] = *e; ++q->count; return PNP_OK;
}
pnp_result_t pnp_queue_pop(pnp_queue_t *q, pnp_graph_event_t *e) {
    if (q == NULL || e == NULL || q->items == NULL || q->capacity == 0u || q->head >= q->capacity || q->count > q->capacity) return PNP_ERR_INVALID_ARGUMENT;
    if (q->count == 0u) return PNP_ERR_QUEUE_EMPTY;
    *e = q->items[q->head]; q->head = (q->head + 1u) % q->capacity; --q->count; return PNP_OK;
}
pnp_result_t pnp_graph_validate(const pnp_graph_t *g, uint32_t queue_capacity) {
    uint32_t i;
    if (g == NULL || queue_capacity == 0u) return PNP_ERR_INVALID_ARGUMENT;
    if (g->node_count > g->node_capacity || g->node_count > PNP_GRAPH_MAX_NODES || g->edge_count > g->edge_capacity) return PNP_ERR_INVALID_GRAPH;
    if (g->state_domain_count > g->state_domain_capacity) return PNP_ERR_INVALID_GRAPH;
    if ((g->node_count != 0u && g->nodes == NULL) || (g->edge_count != 0u && g->edges == NULL) || (g->state_domain_count != 0u && g->state_domains == NULL)) return PNP_ERR_INVALID_GRAPH;
    for (i = 0u; i < g->node_count; ++i) if (pnp_validate_config(&g->nodes[i].config) != PNP_OK || g->nodes[i].state_domain >= g->state_domain_count || g->nodes[i].reserved != 0u) return PNP_ERR_INVALID_GRAPH;
    for (i = 0u; i < g->edge_count; ++i) {
        const pnp_edge_t *e = &g->edges[i];
        if (e->source_node >= g->node_count || e->source_port >= PNP_MAX_OUTPUTS) return PNP_ERR_INVALID_GRAPH;
        if (e->destination_node != PNP_EXTERNAL_NODE && e->destination_node >= g->node_count) return PNP_ERR_INVALID_GRAPH;
        if (e->destination_port >= PNP_GRAPH_INPUT_PORT_COUNT) return PNP_ERR_INVALID_GRAPH;
    }
    return PNP_OK;
}
pnp_result_t pnp_graph_inject(pnp_queue_t *q, uint32_t node_id, uint32_t input_port, const pnp_event_t *event) {
    pnp_graph_event_t ge;
    if (event == NULL || node_id == PNP_EXTERNAL_NODE || input_port >= PNP_GRAPH_INPUT_PORT_COUNT) return PNP_ERR_INVALID_ARGUMENT;
    memset(&ge, 0, sizeof(ge)); ge.node_id = node_id; ge.input_port = input_port; ge.event = *event; ge.event.input_port = input_port;
    return pnp_queue_push(q, &ge);
}
static pnp_result_t emit_external(pnp_graph_output_buffer_t *out, uint32_t node, uint32_t port, const pnp_event_t *event) {
    pnp_graph_output_t *item;
    if (out == NULL) return PNP_OK;
    if (out->count >= out->capacity || out->items == NULL) return PNP_ERR_BUFFER_FULL;
    item = &out->items[out->count++]; memset(item, 0, sizeof(*item)); item->source_node = node; item->port = port; item->event = *event; return PNP_OK;
}
pnp_result_t pnp_graph_run(pnp_graph_t *g, pnp_queue_t *q, pnp_graph_output_buffer_t *external, pnp_run_limits_t limits, pnp_run_stats_t *stats) {
    pnp_run_stats_t local = {0}; pnp_graph_event_t current; pnp_output_buffer_t primitive_outputs; pnp_state_t next_state;
    pnp_result_t r; uint32_t oi, ei;
    if (g == NULL || q == NULL || stats == NULL || (external != NULL && external->count > external->capacity)) return PNP_ERR_INVALID_ARGUMENT;
    r = pnp_graph_validate(g, q->capacity); if (r != PNP_OK) return r;
    local.queue_high_water = q->count;
    while (!pnp_queue_empty(q)) {
        if (local.steps >= limits.max_steps) { *stats = local; return PNP_ERR_STEP_LIMIT; }
        r = pnp_queue_pop(q, &current); if (r != PNP_OK) { *stats = local; return r; }
        if (current.node_id >= g->node_count) { *stats = local; return PNP_ERR_INVALID_GRAPH; }
        current.event.input_port = current.input_port;
        {
            pnp_state_domain_t *domain = &g->state_domains[g->nodes[current.node_id].state_domain];
            r = pnp_execute_valid(&g->nodes[current.node_id].config, &domain->state, &current.event, &next_state, &primitive_outputs);
            if (r == PNP_OK) domain->state = next_state;
        }
        if (r != PNP_OK) { *stats = local; return r; }
        ++local.steps;
        for (oi = 0u; oi < primitive_outputs.count; ++oi) for (ei = 0u; ei < g->edge_count; ++ei) {
            const pnp_edge_t *edge = &g->edges[ei];
            if (edge->source_node == current.node_id && edge->source_port == primitive_outputs.items[oi].port) {
                if (local.emitted_events >= limits.max_emitted_events) { *stats = local; return PNP_ERR_EMIT_LIMIT; }
                if (edge->destination_node == PNP_EXTERNAL_NODE) r = emit_external(external, current.node_id, edge->destination_port, &primitive_outputs.items[oi].event);
                else { pnp_graph_event_t generated; memset(&generated, 0, sizeof(generated)); generated.node_id = edge->destination_node; generated.input_port = edge->destination_port; generated.event = primitive_outputs.items[oi].event; generated.event.input_port = edge->destination_port; r = pnp_queue_push(q, &generated); }
                if (r != PNP_OK) { *stats = local; return r; }
                ++local.emitted_events; if (q->count > local.queue_high_water) local.queue_high_water = q->count;
            }
        }
    }
    *stats = local; return PNP_OK;
}
