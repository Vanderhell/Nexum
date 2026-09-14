#ifndef NEXUM_CONFIG_H
#define NEXUM_CONFIG_H

#include "pnp_primitive.h"

void nexum_cell_config_init(pnp_config_t *config);
pnp_source_t nexum_source_zero(void);
pnp_source_t nexum_source_one(void);
pnp_source_t nexum_source_constant(pnp_word_t value);
pnp_source_t nexum_source_event_type(void);
pnp_source_t nexum_source_metadata(uint8_t index);
pnp_source_t nexum_source_state(uint8_t index);
pnp_source_t nexum_source_sequence(void);
pnp_result_t nexum_cell_set_alu(pnp_config_t *config, pnp_alu_op_t operation,
                                pnp_source_t source_a, pnp_source_t source_b);
pnp_result_t nexum_cell_set_predicate(pnp_config_t *config, pnp_predicate_t predicate,
                                      pnp_source_t compare_source, uint8_t invert,
                                      uint8_t predicate_bit);
pnp_result_t nexum_cell_set_state_update(pnp_config_t *config, pnp_update_mode_t mode,
                                         uint8_t state_index, pnp_word_t constant,
                                         uint8_t event_index);
pnp_result_t nexum_cell_set_event_update(pnp_config_t *config,
                                         pnp_event_destination_t destination,
                                         pnp_event_update_source_t source,
                                         uint8_t destination_index, uint8_t state_index,
                                         pnp_word_t constant);
pnp_result_t nexum_cell_set_emit(pnp_config_t *config, uint32_t emit_true,
                                 uint32_t emit_false);
pnp_result_t nexum_cell_config_validate(const pnp_config_t *config);

#endif
