#ifndef NEXUM_PROGRAM_H
#define NEXUM_PROGRAM_H

#include <stddef.h>
#include "pnp_graph.h"

/* v0.2 IR implementation bounds. These are source-level limits, not a wire ABI. */
#define NEXUM_PROGRAM_MAX_CELLS PNP_GRAPH_MAX_NODES
#define NEXUM_PROGRAM_MAX_STATE_DOMAINS PNP_GRAPH_MAX_NODES
#define NEXUM_PROGRAM_MAX_EDGES 2048u
#define NEXUM_PROGRAM_MAX_INPUTS PNP_GRAPH_INPUT_PORT_COUNT
#define NEXUM_PROGRAM_MAX_OUTPUTS 2048u
#define NEXUM_PROGRAM_FINALIZED UINT32_C(0x4e585031)

typedef struct nexum_cell {
    uint32_t id;
    uint32_t state_domain_id;
    pnp_config_t config;
} nexum_cell_t;

typedef struct nexum_state_domain {
    uint32_t id;
    uint32_t reserved;
    pnp_state_t initial_state;
} nexum_state_domain_t;

typedef struct nexum_edge {
    uint32_t source_cell_id;
    uint32_t source_port;
    uint32_t destination_cell_id;
    uint32_t destination_port;
    /* Explicit FIFO order among all routes from the same cell and port. */
    uint32_t route_order;
    uint32_t reserved;
} nexum_edge_t;

typedef struct nexum_input {
    uint32_t id; /* Unique logical input ID. */
    uint32_t destination_cell_id;
    uint32_t destination_port;
    uint32_t reserved;
} nexum_input_t;

typedef struct nexum_output {
    uint32_t id; /* Unique logical output ID; becomes the runtime boundary port. */
    uint32_t source_cell_id;
    uint32_t source_port;
    uint32_t route_order;
} nexum_output_t;

/*
 * Cell, domain, input, and output IDs are unique. Identical directed edges are
 * permitted when their route_order values differ, matching frozen graph
 * duplicate-edge semantics. A route_order is unique for a source cell/port
 * across both internal edges and output bindings. Declaration array order has
 * no meaning; lowering orders cells/domains by ID and routes by source ID,
 * source port, then route_order.
 */

typedef struct nexum_program {
    uint32_t cell_count;
    uint32_t state_domain_count;
    uint32_t edge_count;
    uint32_t input_count;
    uint32_t output_count;
    uint32_t reserved[3];
    nexum_cell_t cells[NEXUM_PROGRAM_MAX_CELLS];
    nexum_state_domain_t state_domains[NEXUM_PROGRAM_MAX_STATE_DOMAINS];
    nexum_edge_t edges[NEXUM_PROGRAM_MAX_EDGES];
    nexum_input_t inputs[NEXUM_PROGRAM_MAX_INPUTS];
    nexum_output_t outputs[NEXUM_PROGRAM_MAX_OUTPUTS];
} nexum_program_t;

typedef struct nexum_program_requirements {
    uint32_t node_count;
    uint32_t edge_count;
    uint32_t state_domain_count;
    uint32_t input_count;
    size_t node_storage_bytes;
    size_t edge_storage_bytes;
    size_t state_domain_storage_bytes;
    size_t runtime_storage_bytes;
} nexum_program_requirements_t;

typedef struct nexum_runtime_storage {
    pnp_node_t *nodes;
    uint32_t node_capacity;
    pnp_edge_t *edges;
    uint32_t edge_capacity;
    pnp_state_domain_t *state_domains;
    uint32_t state_domain_capacity;
} nexum_runtime_storage_t;

pnp_result_t nexum_program_validate(const nexum_program_t *program);
pnp_result_t nexum_program_finalize(nexum_program_t *program);
pnp_result_t nexum_program_requirements(const nexum_program_t *program,
                                        nexum_program_requirements_t *requirements);
pnp_result_t nexum_program_lower(const nexum_program_t *program,
                                 const nexum_runtime_storage_t *storage,
                                 pnp_graph_t *graph);
pnp_result_t nexum_program_inject(const nexum_program_t *program, pnp_queue_t *queue,
                                  uint32_t input_id, const pnp_event_t *event);

#endif
