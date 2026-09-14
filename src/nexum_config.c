#include <string.h>
#include "nexum_config.h"

static pnp_source_t source(uint8_t kind, uint8_t index, pnp_word_t value) {
    pnp_source_t result; memset(&result, 0, sizeof(result)); result.kind = kind;
    result.index = index; result.immediate = value; return result;
}

void nexum_cell_config_init(pnp_config_t *c) {
    if (c != NULL) { memset(c, 0, sizeof(*c)); c->alu_op = (uint8_t)PNP_ALU_PASS_A; c->predicate = (uint8_t)PNP_PRED_ALWAYS; }
}
pnp_source_t nexum_source_zero(void) { return source((uint8_t)PNP_SRC_ZERO, 0u, 0u); }
pnp_source_t nexum_source_one(void) { return source((uint8_t)PNP_SRC_ONE, 0u, 0u); }
pnp_source_t nexum_source_constant(pnp_word_t v) { return source((uint8_t)PNP_SRC_CONSTANT, 0u, v); }
pnp_source_t nexum_source_event_type(void) { return source((uint8_t)PNP_SRC_TYPE, 0u, 0u); }
pnp_source_t nexum_source_metadata(uint8_t i) { return source((uint8_t)PNP_SRC_EVENT_FIELD, i, 0u); }
pnp_source_t nexum_source_state(uint8_t i) { return source((uint8_t)PNP_SRC_STATE_WORD, i, 0u); }
pnp_source_t nexum_source_sequence(void) { return source((uint8_t)PNP_SRC_SEQUENCE, 0u, 0u); }

pnp_result_t nexum_cell_set_alu(pnp_config_t *c, pnp_alu_op_t op, pnp_source_t a, pnp_source_t b) {
    pnp_config_t next; pnp_result_t result;
    if (c == NULL) return PNP_ERR_INVALID_ARGUMENT;
    next=*c;next.alu_op=(uint8_t)op;next.source_a=a;next.source_b=b;
    result=pnp_validate_config(&next);if(result==PNP_OK)*c=next;return result;
}
pnp_result_t nexum_cell_set_predicate(pnp_config_t *c, pnp_predicate_t p, pnp_source_t s, uint8_t inv, uint8_t bit) {
    pnp_config_t next;pnp_result_t result;if(c==NULL)return PNP_ERR_INVALID_ARGUMENT;
    next=*c;next.predicate=(uint8_t)p;next.compare_source=s;next.predicate_invert=inv;next.predicate_bit=bit;
    result=pnp_validate_config(&next);if(result==PNP_OK)*c=next;return result;
}
pnp_result_t nexum_cell_set_state_update(pnp_config_t *c,pnp_update_mode_t m,uint8_t i,pnp_word_t k,uint8_t ei){pnp_config_t n;pnp_result_t r;if(c==NULL)return PNP_ERR_INVALID_ARGUMENT;n=*c;n.update_mode=(uint8_t)m;n.update_index=i;n.update_constant=k;n.update_event_index=ei;r=pnp_validate_config(&n);if(r==PNP_OK)*c=n;return r;}
pnp_result_t nexum_cell_set_event_update(pnp_config_t *c,pnp_event_destination_t d,pnp_event_update_source_t s,uint8_t di,uint8_t si,pnp_word_t k){pnp_config_t n;pnp_result_t r;if(c==NULL)return PNP_ERR_INVALID_ARGUMENT;n=*c;n.event_update_destination=(uint8_t)d;n.event_update_source=(uint8_t)s;n.event_update_index=di;n.event_update_state_index=si;n.event_update_constant=k;r=pnp_validate_config(&n);if(r==PNP_OK)*c=n;return r;}
pnp_result_t nexum_cell_set_emit(pnp_config_t *c,uint32_t t,uint32_t f){pnp_config_t n;pnp_result_t r;if(c==NULL)return PNP_ERR_INVALID_ARGUMENT;n=*c;n.emit_true=t;n.emit_false=f;r=pnp_validate_config(&n);if(r==PNP_OK)*c=n;return r;}
pnp_result_t nexum_cell_config_validate(const pnp_config_t *c){return pnp_validate_config(c);}
