#ifndef PNP_GRAPH_H
#define PNP_GRAPH_H

#include <stddef.h>
#include "pnp_primitive.h"

#define PNP_EXTERNAL_NODE UINT32_MAX
#define PNP_GRAPH_MAX_NODES 256u

typedef struct pnp_node { pnp_config_t config; uint32_t state_domain; uint32_t reserved; } pnp_node_t;
typedef struct pnp_state_domain { pnp_state_t state; } pnp_state_domain_t;
typedef struct pnp_edge {
    uint32_t source_node;
    uint32_t source_port;
    uint32_t destination_node; /* PNP_EXTERNAL_NODE marks a boundary output. */
    uint32_t destination_port;
} pnp_edge_t; /* Duplicate edges are retained and traversed in array order. */
typedef struct pnp_graph {
    pnp_node_t *nodes; uint32_t node_count; uint32_t node_capacity;
    pnp_edge_t *edges; uint32_t edge_count; uint32_t edge_capacity;
    pnp_state_domain_t *state_domains; uint32_t state_domain_count; uint32_t state_domain_capacity;
} pnp_graph_t;
typedef struct pnp_graph_event { uint32_t node_id; uint32_t input_port; pnp_event_t event; } pnp_graph_event_t;
typedef struct pnp_queue { pnp_graph_event_t *items; uint32_t capacity; uint32_t head; uint32_t count; } pnp_queue_t;
typedef struct pnp_run_limits { uint64_t max_steps; uint64_t max_emitted_events; } pnp_run_limits_t;
typedef struct pnp_run_stats { uint64_t steps; uint64_t emitted_events; uint32_t queue_high_water; uint32_t reserved; } pnp_run_stats_t;
typedef struct pnp_graph_output { uint32_t source_node; uint32_t port; pnp_event_t event; } pnp_graph_output_t;
typedef struct pnp_graph_output_buffer { pnp_graph_output_t *items; uint32_t capacity; uint32_t count; } pnp_graph_output_buffer_t;

typedef enum pnp_d3_status { PNP_D3_NOT_PROVEN = 0, PNP_D3_PROVEN = 1 } pnp_d3_status_t;
typedef struct pnp_graph_analysis { uint32_t node_count, edge_count, maximum_fanout, maximum_out_degree; uint64_t state_bytes, configuration_bytes; uint32_t queue_capacity; uint8_t has_cycles; uint8_t d3_status; uint16_t reserved; } pnp_graph_analysis_t;

pnp_result_t pnp_graph_validate(const pnp_graph_t *graph, uint32_t queue_capacity);
void pnp_queue_init(pnp_queue_t *queue, pnp_graph_event_t *storage, uint32_t capacity);
int pnp_queue_empty(const pnp_queue_t *queue);
int pnp_queue_full(const pnp_queue_t *queue);
uint32_t pnp_queue_count(const pnp_queue_t *queue);
pnp_result_t pnp_queue_push(pnp_queue_t *queue, const pnp_graph_event_t *event);
pnp_result_t pnp_queue_pop(pnp_queue_t *queue, pnp_graph_event_t *event);
pnp_result_t pnp_graph_inject(pnp_queue_t *queue, uint32_t node_id, uint32_t input_port, const pnp_event_t *event);
pnp_result_t pnp_graph_run(pnp_graph_t *graph, pnp_queue_t *queue,
                           pnp_graph_output_buffer_t *outputs, pnp_run_limits_t limits,
                           pnp_run_stats_t *stats);
pnp_result_t pnp_graph_analyze(const pnp_graph_t *graph, uint32_t queue_capacity, pnp_graph_analysis_t *analysis);

#endif
