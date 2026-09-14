#include <limits.h>
#include "test_support.h"

static int test_event_destinations(void){
    pnp_config_t c=pass_config(3u);pnp_state_t s={{0}},next;pnp_event_t e={0},before;pnp_output_buffer_t o;
    e.type=7u;e.flags=8u;e.sequence=9u;e.epoch=10u;e.fields[0]=11u;e.fields[PNP_EVENT_FIELD_COUNT-1u]=12u;before=e;
    c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=UINT64_C(0x12345678);
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.type==UINT32_C(0x12345678));
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_FLAGS;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.flags==UINT64_C(0x12345678));
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_SEQUENCE;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.sequence==UINT64_C(0x12345678));
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_EPOCH;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.epoch==UINT64_C(0x12345678));
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_index=0u;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.fields[0]==UINT64_C(0x12345678));
    c.event_update_index=(uint8_t)(PNP_EVENT_FIELD_COUNT-1u);CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&o.items[0].event.fields[PNP_EVENT_FIELD_COUNT-1u]==UINT64_C(0x12345678));
    CHECK(o.count==2u&&memcmp(&o.items[0].event,&o.items[1].event,sizeof(e))==0);CHECK(memcmp(&e,&before,sizeof(e))==0);
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_NONE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;c.event_update_index=0u;CHECK(pnp_execute(&c,&s,&e,&next,&o)==PNP_OK&&memcmp(&o.items[0].event,&e,sizeof(e))==0);
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_COUNT;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_index=(uint8_t)PNP_EVENT_FIELD_COUNT;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);
    return 0;
}

static int expect_result(pnp_config_t *c,pnp_state_t *s,pnp_event_t *e,pnp_word_t value){
    pnp_state_t out; pnp_output_buffer_t outputs; c->emit_true=1u;c->event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c->event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;c->event_update_index=0u;
    CHECK(pnp_execute(c,s,e,&out,&outputs)==PNP_OK);CHECK(outputs.count==1u);CHECK(outputs.items[0].event.fields[0]==value);return 0;
}
int main(void){
    pnp_config_t c=pass_config(0u);pnp_state_t s={{0}};pnp_event_t e={0};pnp_state_t out; pnp_output_buffer_t outputs;uint32_t i;
    _Static_assert(sizeof(pnp_word_t)==8u,"word size");_Static_assert(PNP_STATE_WORD_COUNT==8u,"state words");_Static_assert(PNP_MAX_OUTPUTS==8u,"outputs");CHECK(pnp_validate_config(&c)==PNP_OK);
    c.source_a.kind=(uint8_t)PNP_SRC_TYPE;e.type=UINT32_C(0xfedcba98);c.emit_true=1u;c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.items[0].event.fields[0]==UINT32_C(0xfedcba98));c.source_a.index=1u;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);c.source_a.index=0u;
    c.source_a.kind=(uint8_t)PNP_SRC_CONSTANT;c.source_a.immediate=UINT64_MAX;c.source_b.kind=(uint8_t)PNP_SRC_ONE;
    c.alu_op=(uint8_t)PNP_ALU_ADD;CHECK(expect_result(&c,&s,&e,0u)==0);c.alu_op=(uint8_t)PNP_ALU_SUB;CHECK(expect_result(&c,&s,&e,UINT64_MAX-1u)==0);
    c.source_a.immediate=UINT64_C(0xaa);c.source_b.kind=(uint8_t)PNP_SRC_CONSTANT;c.source_b.immediate=UINT64_C(0x0f);
    c.alu_op=(uint8_t)PNP_ALU_AND;CHECK(expect_result(&c,&s,&e,UINT64_C(0x0a))==0);c.alu_op=(uint8_t)PNP_ALU_OR;CHECK(expect_result(&c,&s,&e,UINT64_C(0xaf))==0);c.alu_op=(uint8_t)PNP_ALU_XOR;CHECK(expect_result(&c,&s,&e,UINT64_C(0xa5))==0);
    c.source_a.immediate=1u;c.source_b.immediate=63u;c.alu_op=(uint8_t)PNP_ALU_SHL;CHECK(expect_result(&c,&s,&e,UINT64_C(1)<<63u)==0);c.source_b.immediate=64u;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);c.source_b.kind=(uint8_t)PNP_SRC_EVENT_FIELD;c.source_b.index=0u;e.fields[0]=64u;CHECK(expect_result(&c,&s,&e,1u)==0);c.source_a.immediate=UINT64_MAX;c.alu_op=(uint8_t)PNP_ALU_SHR;e.fields[0]=63u;CHECK(expect_result(&c,&s,&e,1u)==0);
    c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;c.source_a.index=7u;e.fields[7]=9u;c.predicate=(uint8_t)PNP_PRED_EQ;c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=9u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.count==1u);
    c.predicate_invert=1u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.count==0u);c.predicate_invert=0u;c.predicate=(uint8_t)PNP_PRED_LT_U;c.compare_source.immediate=10u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.count==1u);
    c.predicate=(uint8_t)PNP_PRED_BIT_SET;c.predicate_bit=63u;e.fields[7]=UINT64_C(1)<<63u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.count==1u);
    c.predicate=(uint8_t)PNP_PRED_ALWAYS;c.predicate_bit=0u;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.update_index=7u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&out.words[7]==e.fields[7]);CHECK(memcmp(&s,&(pnp_state_t){{0}},sizeof(s))==0);
    c.emit_true=UINT32_C(0xff);c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_index=7u;c.event_update_constant=42u;CHECK(pnp_execute(&c,&s,&e,&out,&outputs)==PNP_OK&&outputs.count==PNP_MAX_OUTPUTS);for(i=0u;i<outputs.count;++i){CHECK(outputs.items[i].port==i);CHECK(outputs.items[i].event.fields[7]==42u);CHECK(memcmp(&outputs.items[0].event,&outputs.items[i].event,sizeof(e))==0);}
    c.source_a.kind=(uint8_t)PNP_SRC_COUNT;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);c.source_a.kind=(uint8_t)PNP_SRC_ZERO;c.emit_true=UINT32_C(0x100);CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);c.emit_true=0u;c.update_index=PNP_STATE_WORD_COUNT;CHECK(pnp_validate_config(&c)==PNP_ERR_INVALID_CONFIG);
    CHECK(pnp_execute(NULL,&s,&e,&out,&outputs)==PNP_ERR_INVALID_ARGUMENT);CHECK(test_event_destinations()==0);return 0;
}
