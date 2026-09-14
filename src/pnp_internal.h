#ifndef PNP_INTERNAL_H
#define PNP_INTERNAL_H
#include "pnp_primitive.h"
pnp_word_t pnp_read_source(const pnp_source_t *source, const pnp_state_t *state, const pnp_event_t *event);
pnp_result_t pnp_execute_valid(const pnp_config_t *config, const pnp_state_t *state_in, const pnp_event_t *event_in, pnp_state_t *state_out, pnp_output_buffer_t *outputs);
#endif
