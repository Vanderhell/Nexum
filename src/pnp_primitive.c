#include <string.h>
#include "pnp_internal.h"

pnp_word_t pnp_read_source(const pnp_source_t *s, const pnp_state_t *state, const pnp_event_t *event) {
    switch ((pnp_source_kind_t)s->kind) {
        case PNP_SRC_ZERO: return UINT64_C(0); case PNP_SRC_ONE: return UINT64_C(1);
        case PNP_SRC_CONSTANT: return s->immediate; case PNP_SRC_EVENT_FIELD: return event->fields[s->index];
        case PNP_SRC_STATE_WORD: return state->words[s->index]; case PNP_SRC_TYPE: return event->type; case PNP_SRC_SEQUENCE: return event->sequence;
        case PNP_SRC_EPOCH: return event->epoch; case PNP_SRC_FLAGS: return event->flags;
        case PNP_SRC_INPUT_PORT: return event->input_port; default: return UINT64_C(0);
    }
}
static pnp_word_t alu(uint8_t op, pnp_word_t a, pnp_word_t b) {
    switch ((pnp_alu_op_t)op) {
        case PNP_ALU_PASS_A: return a; case PNP_ALU_ADD: return a + b; case PNP_ALU_SUB: return a - b;
        case PNP_ALU_AND: return a & b; case PNP_ALU_OR: return a | b; case PNP_ALU_XOR: return a ^ b;
        case PNP_ALU_SHL: return a << (b & UINT64_C(63)); case PNP_ALU_SHR: return a >> (b & UINT64_C(63));
        default: return UINT64_C(0);
    }
}
static int pred(const pnp_config_t *c, pnp_word_t result, pnp_word_t compare) {
    int value = 0;
    switch ((pnp_predicate_t)c->predicate) {
        case PNP_PRED_ALWAYS: value = 1; break; case PNP_PRED_EQ: value = result == compare; break;
        case PNP_PRED_LT_U: value = result < compare; break;
        case PNP_PRED_BIT_SET: value = ((result >> c->predicate_bit) & UINT64_C(1)) != 0u; break;
        default: break;
    }
    return value != (c->predicate_invert != 0u);
}
pnp_result_t pnp_execute_valid(const pnp_config_t *c, const pnp_state_t *state_in, const pnp_event_t *event_in, pnp_state_t *state_out, pnp_output_buffer_t *outputs) {
    pnp_word_t a, b, result, compare, update_value = 0u, mutation_value = 0u;
    uint32_t mask, port; int gate;
    *state_out = *state_in; memset(outputs, 0, sizeof(*outputs));
    a = pnp_read_source(&c->source_a, state_in, event_in); b = pnp_read_source(&c->source_b, state_in, event_in);
    result = alu(c->alu_op, a, b); compare = pnp_read_source(&c->compare_source, state_in, event_in); gate = pred(c, result, compare);
    switch ((pnp_update_mode_t)c->update_mode) {
        case PNP_UPDATE_NONE: break; case PNP_UPDATE_RESULT: update_value = result; break;
        case PNP_UPDATE_OPERAND_A: update_value = a; break; case PNP_UPDATE_OPERAND_B: update_value = b; break;
        case PNP_UPDATE_CONSTANT: update_value = c->update_constant; break;
        case PNP_UPDATE_EVENT_FIELD: update_value = event_in->fields[c->update_event_index]; break; default: break;
    }
    if (gate && c->update_mode != (uint8_t)PNP_UPDATE_NONE) state_out->words[c->update_index] = update_value;
    switch ((pnp_event_update_source_t)c->event_update_source) {
        case PNP_EVENT_VALUE_RESULT: mutation_value = result; break;
        case PNP_EVENT_VALUE_OPERAND_A: mutation_value = a; break;
        case PNP_EVENT_VALUE_OPERAND_B: mutation_value = b; break;
        case PNP_EVENT_VALUE_STATE_WORD: mutation_value = state_out->words[c->event_update_state_index]; break;
        case PNP_EVENT_VALUE_CONSTANT: mutation_value = c->event_update_constant; break;
        default: break;
    }
    mask = gate ? c->emit_true : c->emit_false;
    for (port = 0u; port < PNP_MAX_OUTPUTS; ++port) if ((mask & (UINT32_C(1) << port)) != 0u) {
        pnp_output_t *out = &outputs->items[outputs->count++]; out->port = port; out->event = *event_in;
        switch ((pnp_event_destination_t)c->event_update_destination) {
            case PNP_EVENT_DEST_NONE: break;
            case PNP_EVENT_DEST_TYPE: out->event.type = (uint32_t)mutation_value; break;
            case PNP_EVENT_DEST_FLAGS: out->event.flags = mutation_value; break;
            case PNP_EVENT_DEST_SEQUENCE: out->event.sequence = mutation_value; break;
            case PNP_EVENT_DEST_EPOCH: out->event.epoch = mutation_value; break;
            case PNP_EVENT_DEST_METADATA: out->event.fields[c->event_update_index] = mutation_value; break;
            default: break;
        }
    }
    return PNP_OK;
}
pnp_result_t pnp_execute(const pnp_config_t *c, const pnp_state_t *state_in, const pnp_event_t *event_in, pnp_state_t *state_out, pnp_output_buffer_t *outputs) {
    pnp_result_t r;
    if (c == NULL || state_in == NULL || event_in == NULL || state_out == NULL || outputs == NULL) return PNP_ERR_INVALID_ARGUMENT;
    r = pnp_validate_config(c);
    if (r != PNP_OK) { memset(state_out, 0, sizeof(*state_out)); memset(outputs, 0, sizeof(*outputs)); return r; }
    return pnp_execute_valid(c, state_in, event_in, state_out, outputs);
}
