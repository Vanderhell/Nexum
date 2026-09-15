#include <string.h>
#include "nexum_builder.h"

static pnp_result_t fail(nexum_builder_t *b, pnp_result_t error) {
    if (b != NULL && b->error == PNP_OK) b->error = error;
    return error;
}

static pnp_result_t ready(nexum_builder_t *b) {
    if (b == NULL || b->program == NULL) return PNP_ERR_INVALID_ARGUMENT;
    if (b->error != PNP_OK) return b->error;
    if (b->finalized != 0u) return PNP_ERR_INVALID_ARGUMENT;
    return PNP_OK;
}

pnp_result_t nexum_builder_init(nexum_builder_t *b, nexum_program_t *p,
                                nexum_builder_capacities_t c) {
    if (b == NULL || p == NULL) return PNP_ERR_INVALID_ARGUMENT;
    if (c.cells > NEXUM_PROGRAM_MAX_CELLS || c.state_domains > NEXUM_PROGRAM_MAX_STATE_DOMAINS ||
        c.edges > NEXUM_PROGRAM_MAX_EDGES || c.inputs > NEXUM_PROGRAM_MAX_INPUTS ||
        c.outputs > NEXUM_PROGRAM_MAX_OUTPUTS) return PNP_ERR_INVALID_ARGUMENT;
    memset(p, 0, sizeof(*p)); memset(b, 0, sizeof(*b)); b->program = p; b->capacities = c;
    return PNP_OK;
}

pnp_result_t nexum_builder_add_state_domain(nexum_builder_t *b, const pnp_state_t *state,
                                            nexum_domain_ref_t *domain) {
    nexum_state_domain_t *item; pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    if (domain == NULL) return fail(b, PNP_ERR_INVALID_ARGUMENT);
    if (b->program->state_domain_count >= b->capacities.state_domains) return fail(b, PNP_ERR_BUFFER_FULL);
    item = &b->program->state_domains[b->program->state_domain_count]; memset(item, 0, sizeof(*item));
    item->id = b->program->state_domain_count; if (state != NULL) item->initial_state = *state;
    *domain = item->id; ++b->program->state_domain_count; return PNP_OK;
}

pnp_result_t nexum_builder_add_cell(nexum_builder_t *b, const pnp_config_t *config,
                                    nexum_domain_ref_t domain, nexum_cell_ref_t *cell) {
    nexum_cell_t *item; pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    if (config == NULL || cell == NULL || pnp_validate_config(config) != PNP_OK)
        return fail(b, PNP_ERR_INVALID_CONFIG);
    if (domain >= b->program->state_domain_count) return fail(b, PNP_ERR_INVALID_CONFIG);
    if (b->program->cell_count >= b->capacities.cells) return fail(b, PNP_ERR_BUFFER_FULL);
    item = &b->program->cells[b->program->cell_count]; item->id = b->program->cell_count;
    item->state_domain_id = domain; item->config = *config; *cell = item->id;
    ++b->program->cell_count; return PNP_OK;
}

static uint32_t next_order(const nexum_program_t *p, uint32_t cell, uint32_t port) {
    uint32_t i, result = 0u;
    for (i = 0u; i < p->edge_count; ++i) if (p->edges[i].source_cell_id == cell && p->edges[i].source_port == port) ++result;
    for (i = 0u; i < p->output_count; ++i) if (p->outputs[i].source_cell_id == cell && p->outputs[i].source_port == port) ++result;
    return result;
}

pnp_result_t nexum_builder_connect(nexum_builder_t *b, nexum_cell_ref_t source,
                                   uint32_t source_port, nexum_cell_ref_t destination,
                                   uint32_t destination_port) {
    nexum_edge_t *item; pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    if (source >= b->program->cell_count || destination >= b->program->cell_count ||
        source_port >= PNP_MAX_OUTPUTS || destination_port >= PNP_GRAPH_INPUT_PORT_COUNT)
        return fail(b, PNP_ERR_INVALID_GRAPH);
    if (b->program->edge_count >= b->capacities.edges) return fail(b, PNP_ERR_BUFFER_FULL);
    item = &b->program->edges[b->program->edge_count];
    *item = (nexum_edge_t){source, source_port, destination, destination_port,
                           next_order(b->program, source, source_port), 0u};
    ++b->program->edge_count; return PNP_OK;
}

pnp_result_t nexum_builder_bind_input(nexum_builder_t *b, nexum_cell_ref_t destination,
                                      uint32_t destination_port, uint32_t *input_id) {
    nexum_input_t *item; pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    if (input_id == NULL || destination >= b->program->cell_count || destination_port >= PNP_GRAPH_INPUT_PORT_COUNT)
        return fail(b, PNP_ERR_INVALID_GRAPH);
    if (b->program->input_count >= b->capacities.inputs) return fail(b, PNP_ERR_BUFFER_FULL);
    item = &b->program->inputs[b->program->input_count];
    *item = (nexum_input_t){b->program->input_count, destination, destination_port, 0u};
    *input_id = item->id; ++b->program->input_count; return PNP_OK;
}

pnp_result_t nexum_builder_bind_output(nexum_builder_t *b, nexum_cell_ref_t source,
                                       uint32_t source_port, uint32_t *output_id) {
    nexum_output_t *item; pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    if (output_id == NULL || source >= b->program->cell_count || source_port >= PNP_MAX_OUTPUTS)
        return fail(b, PNP_ERR_INVALID_GRAPH);
    if (b->program->output_count >= b->capacities.outputs || b->program->output_count >= PNP_GRAPH_INPUT_PORT_COUNT)
        return fail(b, PNP_ERR_BUFFER_FULL);
    item = &b->program->outputs[b->program->output_count];
    *item = (nexum_output_t){b->program->output_count, source, source_port,
                             next_order(b->program, source, source_port)};
    *output_id = item->id; ++b->program->output_count; return PNP_OK;
}

pnp_result_t nexum_builder_finalize(nexum_builder_t *b) {
    pnp_result_t r = ready(b);
    if (r != PNP_OK) return r;
    r = nexum_program_finalize(b->program); if (r != PNP_OK) return fail(b, r);
    b->finalized = 1u; return PNP_OK;
}
