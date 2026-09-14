#ifndef NEXUM_BUILDER_H
#define NEXUM_BUILDER_H

#include "nexum_program.h"

typedef uint32_t nexum_domain_ref_t;
typedef uint32_t nexum_cell_ref_t;

#define NEXUM_INVALID_REF UINT32_MAX

typedef struct nexum_builder_capacities {
    uint32_t cells;
    uint32_t state_domains;
    uint32_t edges;
    uint32_t inputs;
    uint32_t outputs;
} nexum_builder_capacities_t;

typedef struct nexum_builder {
    nexum_program_t *program;
    nexum_builder_capacities_t capacities;
    pnp_result_t error;
    uint8_t finalized;
    uint8_t reserved[3];
} nexum_builder_t;

pnp_result_t nexum_builder_init(nexum_builder_t *builder, nexum_program_t *program,
                                nexum_builder_capacities_t capacities);
pnp_result_t nexum_builder_add_state_domain(nexum_builder_t *builder,
                                            const pnp_state_t *initial_state,
                                            nexum_domain_ref_t *domain);
pnp_result_t nexum_builder_add_cell(nexum_builder_t *builder, const pnp_config_t *config,
                                    nexum_domain_ref_t domain, nexum_cell_ref_t *cell);
pnp_result_t nexum_builder_connect(nexum_builder_t *builder, nexum_cell_ref_t source,
                                   uint32_t source_port, nexum_cell_ref_t destination,
                                   uint32_t destination_port);
pnp_result_t nexum_builder_bind_input(nexum_builder_t *builder, nexum_cell_ref_t destination,
                                      uint32_t destination_port, uint32_t *input_id);
pnp_result_t nexum_builder_bind_output(nexum_builder_t *builder, nexum_cell_ref_t source,
                                       uint32_t source_port, uint32_t *output_id);
pnp_result_t nexum_builder_finalize(nexum_builder_t *builder);

#endif
