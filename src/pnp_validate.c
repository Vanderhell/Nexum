#include <stddef.h>
#include "pnp_primitive.h"

static int source_valid(const pnp_source_t *s) {
    if (s->kind >= (uint8_t)PNP_SRC_COUNT || s->reserved != 0u) return 0;
    if (s->kind == (uint8_t)PNP_SRC_EVENT_FIELD) return s->index < PNP_EVENT_FIELD_COUNT;
    if (s->kind == (uint8_t)PNP_SRC_STATE_WORD) return s->index < PNP_STATE_WORD_COUNT;
    return s->index == 0u;
}

pnp_result_t pnp_validate_config(const pnp_config_t *c) {
    const uint32_t valid_mask = (UINT32_C(1) << PNP_MAX_OUTPUTS) - UINT32_C(1);
    if (c == NULL) return PNP_ERR_INVALID_ARGUMENT;
    if (!source_valid(&c->source_a) || !source_valid(&c->source_b) || !source_valid(&c->compare_source)) return PNP_ERR_INVALID_CONFIG;
    if (c->alu_op >= (uint8_t)PNP_ALU_COUNT || c->predicate >= (uint8_t)PNP_PRED_COUNT || c->predicate_invert > 1u) return PNP_ERR_INVALID_CONFIG;
    if ((c->alu_op == (uint8_t)PNP_ALU_SHL || c->alu_op == (uint8_t)PNP_ALU_SHR) && c->source_b.kind == (uint8_t)PNP_SRC_CONSTANT && c->source_b.immediate >= UINT64_C(64)) return PNP_ERR_INVALID_CONFIG;
    if (c->predicate == (uint8_t)PNP_PRED_BIT_SET && c->predicate_bit >= 64u) return PNP_ERR_INVALID_CONFIG;
    if (c->predicate != (uint8_t)PNP_PRED_BIT_SET && c->predicate_bit != 0u) return PNP_ERR_INVALID_CONFIG;
    if (c->update_mode >= (uint8_t)PNP_UPDATE_COUNT || c->event_update_destination >= (uint8_t)PNP_EVENT_DEST_COUNT || c->event_update_source >= (uint8_t)PNP_EVENT_VALUE_COUNT) return PNP_ERR_INVALID_CONFIG;
    if (c->update_mode == (uint8_t)PNP_UPDATE_NONE) { if (c->update_index != 0u || c->update_event_index != 0u) return PNP_ERR_INVALID_CONFIG; }
    else if (c->update_index >= PNP_STATE_WORD_COUNT) return PNP_ERR_INVALID_CONFIG;
    if (c->update_mode == (uint8_t)PNP_UPDATE_EVENT_FIELD) { if (c->update_event_index >= PNP_EVENT_FIELD_COUNT) return PNP_ERR_INVALID_CONFIG; }
    else if (c->update_event_index != 0u) return PNP_ERR_INVALID_CONFIG;
    if (c->event_update_destination == (uint8_t)PNP_EVENT_DEST_NONE) {
        if (c->event_update_source != (uint8_t)PNP_EVENT_VALUE_RESULT || c->event_update_index != 0u || c->event_update_state_index != 0u) return PNP_ERR_INVALID_CONFIG;
    } else if (c->event_update_destination == (uint8_t)PNP_EVENT_DEST_METADATA) {
        if (c->event_update_index >= PNP_EVENT_FIELD_COUNT) return PNP_ERR_INVALID_CONFIG;
    } else if (c->event_update_index != 0u) return PNP_ERR_INVALID_CONFIG;
    if (c->event_update_source == (uint8_t)PNP_EVENT_VALUE_STATE_WORD) {
        if (c->event_update_state_index >= PNP_STATE_WORD_COUNT) return PNP_ERR_INVALID_CONFIG;
    } else if (c->event_update_state_index != 0u) return PNP_ERR_INVALID_CONFIG;
    if (((c->emit_true | c->emit_false) & ~valid_mask) != 0u || c->reserved != 0u) return PNP_ERR_INVALID_CONFIG;
    return PNP_OK;
}
