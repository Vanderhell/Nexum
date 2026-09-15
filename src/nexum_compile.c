#include <string.h>
#include "nexum_compile.h"

static pnp_result_t spec_valid(const nexum_compile_spec_t *s) {
    if (s == NULL || s->behavior >= (uint8_t)NEXUM_BEHAVIOR_COUNT || s->reserved != 0u)
        return PNP_ERR_INVALID_ARGUMENT;
    if ((s->behavior == (uint8_t)NEXUM_BEHAVIOR_CONDITIONAL_DROP ||
         s->behavior == (uint8_t)NEXUM_BEHAVIOR_CONDITIONAL_ROUTE ||
         s->behavior == (uint8_t)NEXUM_BEHAVIOR_REWRITE_METADATA) &&
        s->metadata_index >= PNP_EVENT_FIELD_COUNT) return PNP_ERR_INVALID_CONFIG;
    if (s->behavior == (uint8_t)NEXUM_BEHAVIOR_COUNTER && s->state_index >= PNP_STATE_WORD_COUNT)
        return PNP_ERR_INVALID_CONFIG;
    if (s->behavior == (uint8_t)NEXUM_BEHAVIOR_RETRY && s->retry_limit == UINT32_MAX)
        return PNP_ERR_INVALID_CONFIG;
    return PNP_OK;
}

pnp_result_t nexum_compile_requirements(const nexum_compile_spec_t *s,
                                        nexum_compile_requirements_t *r) {
    pnp_result_t result;
    if (r == NULL) return PNP_ERR_INVALID_ARGUMENT;
    result = spec_valid(s); if (result != PNP_OK) return result;
    memset(r, 0, sizeof(*r)); r->exact = 1u; r->capacities.state_domains = 1u;
    r->capacities.inputs = 1u;
    if (s->behavior == (uint8_t)NEXUM_BEHAVIOR_RETRY) {
        r->capacities.cells=12u;r->capacities.edges=12u;r->capacities.outputs=3u;
    } else {
        r->capacities.cells=1u;r->capacities.edges=0u;
        r->capacities.outputs = s->behavior == (uint8_t)NEXUM_BEHAVIOR_CONDITIONAL_ROUTE ? 2u : 1u;
    }
    return PNP_OK;
}

static pnp_config_t base(uint32_t emit) {
    pnp_config_t c; nexum_cell_config_init(&c); c.emit_true=emit; return c;
}
static pnp_config_t type_match(uint32_t type) {
    pnp_config_t c=base(1u);c.source_a=nexum_source_event_type();c.predicate=(uint8_t)PNP_PRED_EQ;
    c.compare_source=nexum_source_constant(type);c.emit_false=2u;return c;
}
static pnp_config_t constant_write(uint8_t index,pnp_word_t value,uint32_t emit) {
    pnp_config_t c=base(emit);c.update_mode=(uint8_t)PNP_UPDATE_CONSTANT;c.update_index=index;c.update_constant=value;return c;
}

static pnp_result_t compile_single(const nexum_compile_spec_t *s,nexum_builder_t *b,nexum_domain_ref_t d) {
    pnp_config_t c=base(1u);nexum_cell_ref_t cell;uint32_t id;pnp_result_t r;
    switch((nexum_behavior_t)s->behavior){
        case NEXUM_BEHAVIOR_PASS:break;
        case NEXUM_BEHAVIOR_CONDITIONAL_DROP:c.source_a=nexum_source_metadata(s->metadata_index);c.predicate=(uint8_t)PNP_PRED_EQ;c.compare_source=nexum_source_constant(s->match_value);break;
        case NEXUM_BEHAVIOR_CONDITIONAL_ROUTE:c.source_a=nexum_source_metadata(s->metadata_index);c.predicate=(uint8_t)PNP_PRED_EQ;c.compare_source=nexum_source_constant(s->match_value);c.emit_false=2u;break;
        case NEXUM_BEHAVIOR_REWRITE_METADATA:c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_index=s->metadata_index;c.event_update_constant=s->rewrite_value;break;
        case NEXUM_BEHAVIOR_REWRITE_TYPE:c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=s->rewrite_value;break;
        case NEXUM_BEHAVIOR_COUNTER:c.source_a=nexum_source_state(s->state_index);c.source_b=nexum_source_one();c.alu_op=(uint8_t)PNP_ALU_ADD;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.update_index=s->state_index;break;
        case NEXUM_BEHAVIOR_ACK:c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=s->rewrite_value;break;
        default:return PNP_ERR_INVALID_ARGUMENT;
    }
    r=nexum_builder_add_cell(b,&c,d,&cell);if(r!=PNP_OK)return r;
    r=nexum_builder_bind_input(b,cell,0u,&id);if(r!=PNP_OK)return r;
    r=nexum_builder_bind_output(b,cell,0u,&id);if(r!=PNP_OK)return r;
    if(s->behavior==(uint8_t)NEXUM_BEHAVIOR_CONDITIONAL_ROUTE)r=nexum_builder_bind_output(b,cell,1u,&id);
    return r;
}

static pnp_result_t compile_retry(const nexum_compile_spec_t *s,nexum_builder_t *b,nexum_domain_ref_t d) {
    pnp_config_t c[12];nexum_cell_ref_t n[12];uint32_t id,i;pnp_result_t r;
    c[0]=type_match(s->data_type);
    c[1]=base(1u);c[1].source_a=nexum_source_sequence();c[1].update_mode=(uint8_t)PNP_UPDATE_RESULT;c[1].update_index=0u;
    c[2]=constant_write(1u,1u,1u);c[3]=constant_write(2u,0u,1u);c[4]=type_match(s->time_type);
    c[5]=base(1u);c[5].source_a=nexum_source_state(1u);c[5].predicate=(uint8_t)PNP_PRED_EQ;c[5].compare_source=nexum_source_one();
    c[6]=base(1u);c[6].source_a=nexum_source_state(0u);c[6].event_update_destination=(uint8_t)PNP_EVENT_DEST_SEQUENCE;c[6].event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;
    c[7]=base(1u);c[7].source_a=nexum_source_state(2u);c[7].source_b=nexum_source_one();c[7].alu_op=(uint8_t)PNP_ALU_ADD;c[7].predicate=(uint8_t)PNP_PRED_LT_U;c[7].compare_source=nexum_source_constant((pnp_word_t)s->retry_limit+1u);c[7].update_mode=(uint8_t)PNP_UPDATE_RESULT;c[7].update_index=2u;c[7].emit_false=2u;c[7].event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c[7].event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c[7].event_update_constant=s->retry_type;
    c[8]=type_match(s->ack_type);c[8].emit_false=0u;
    c[9]=base(1u);c[9].event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c[9].event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c[9].event_update_constant=s->failure_type;
    c[10]=constant_write(1u,0u,0u);
    c[11]=base(1u);c[11].source_a=nexum_source_sequence();c[11].predicate=(uint8_t)PNP_PRED_EQ;c[11].compare_source=nexum_source_state(0u);c[11].emit_false=0u;
    for(i=0u;i<12u;++i){r=nexum_builder_add_cell(b,&c[i],d,&n[i]);if(r!=PNP_OK)return r;}
#define CONNECT(a,p,z) do{r=nexum_builder_connect(b,n[a],p,n[z],0u);if(r!=PNP_OK)return r;}while(0)
    CONNECT(0,0u,1);CONNECT(0,1u,4);CONNECT(1,0u,2);CONNECT(2,0u,3);
    CONNECT(4,0u,5);CONNECT(4,1u,8);CONNECT(5,0u,6);CONNECT(6,0u,7);CONNECT(7,1u,9);CONNECT(8,0u,11);CONNECT(11,0u,10);CONNECT(9,0u,10);
#undef CONNECT
    r=nexum_builder_bind_input(b,n[0],0u,&id);if(r!=PNP_OK)return r;
    r=nexum_builder_bind_output(b,n[3],0u,&id);if(r!=PNP_OK)return r;
    r=nexum_builder_bind_output(b,n[7],0u,&id);if(r!=PNP_OK)return r;
    return nexum_builder_bind_output(b,n[9],0u,&id);
}

pnp_result_t nexum_compile(const nexum_compile_spec_t *s,nexum_builder_capacities_t capacities,nexum_program_t *p) {
    nexum_compile_requirements_t req;nexum_builder_t b;nexum_domain_ref_t d;pnp_result_t r;
    r=nexum_compile_requirements(s,&req);if(r!=PNP_OK)return r;
    if(capacities.cells<req.capacities.cells||capacities.state_domains<req.capacities.state_domains||capacities.edges<req.capacities.edges||capacities.inputs<req.capacities.inputs||capacities.outputs<req.capacities.outputs)return PNP_ERR_BUFFER_FULL;
    r=nexum_builder_init(&b,p,capacities);if(r!=PNP_OK)return r;
    r=nexum_builder_add_state_domain(&b,NULL,&d);if(r!=PNP_OK)return r;
    r=s->behavior==(uint8_t)NEXUM_BEHAVIOR_RETRY?compile_retry(s,&b,d):compile_single(s,&b,d);
    if(r!=PNP_OK)return r;
    return nexum_builder_finalize(&b);
}
