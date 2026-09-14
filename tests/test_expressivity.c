#include "test_support.h"

#define TEST_EVENT_ACK UINT32_C(0x41434b00)
#define TEST_EVENT_TIME UINT32_C(0x54494d45)
#define TEST_EVENT_RETRY UINT32_C(0x52545259)

static int run(pnp_config_t *c,pnp_state_t *s,pnp_event_t *e,pnp_state_t *next,pnp_output_buffer_t *o){return pnp_execute(c,s,e,next,o)==PNP_OK?0:1;}

static int ack_expressivity(void){
    pnp_config_t c=pass_config(UINT32_C(1)<<3u);pnp_state_t s={{0}},next;pnp_event_t input={0},before;pnp_output_buffer_t o;
    input.type=17u;input.sequence=UINT64_C(0x1122334455667788);input.fields[0]=UINT64_C(0xaabbccdd);before=input;
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=TEST_EVENT_ACK;
    CHECK(run(&c,&s,&input,&next,&o)==0);CHECK(o.count==1u&&o.items[0].port==3u);CHECK(o.items[0].event.type==TEST_EVENT_ACK);CHECK(o.items[0].event.sequence==input.sequence);CHECK(o.items[0].event.fields[0]==input.fields[0]);CHECK(memcmp(&input,&before,sizeof(input))==0);return 0;
}

static int retry_type_expressivity(void){
    pnp_config_t c=pass_config(1u);pnp_state_t s={{0}},next;pnp_event_t tick={0};pnp_output_buffer_t o;
    tick.type=TEST_EVENT_TIME;c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_b.kind=(uint8_t)PNP_SRC_ONE;c.alu_op=(uint8_t)PNP_ALU_ADD;c.predicate=(uint8_t)PNP_PRED_LT_U;c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=3u;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.emit_false=2u;c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=TEST_EVENT_RETRY;
    CHECK(run(&c,&s,&tick,&next,&o)==0&&next.words[0]==1u&&o.items[0].event.type==TEST_EVENT_RETRY&&o.items[0].port==0u);s=next;
    CHECK(run(&c,&s,&tick,&next,&o)==0&&next.words[0]==2u&&o.items[0].event.type==TEST_EVENT_RETRY&&o.items[0].port==0u);s=next;
    CHECK(run(&c,&s,&tick,&next,&o)==0&&next.words[0]==2u&&o.items[0].event.type==TEST_EVENT_RETRY&&o.items[0].port==1u);return 0;
}

int main(void){pnp_config_t c;pnp_state_t s={{0}},next;pnp_event_t e={0};pnp_output_buffer_t o;
    c=pass_config(1u);CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==1u);c.emit_true=0u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==0u);
    c=pass_config(2u);CHECK(run(&c,&s,&e,&next,&o)==0&&o.items[0].port==1u);c.emit_true=UINT32_C(0xff);CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==8u);
    c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;c.predicate=(uint8_t)PNP_PRED_LT_U;c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=10u;c.emit_false=2u;e.fields[0]=9u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.items[0].port==0u);e.fields[0]=10u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.items[0].port==1u);
    c=pass_config(1u);c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=99u;c.event_update_index=2u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.items[0].event.fields[2]==99u);
    c=pass_config(0u);c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_b.kind=(uint8_t)PNP_SRC_ONE;c.alu_op=(uint8_t)PNP_ALU_ADD;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;CHECK(run(&c,&s,&e,&next,&o)==0&&next.words[0]==1u);s=next;CHECK(run(&c,&s,&e,&next,&o)==0&&next.words[0]==2u);
    c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_SEQUENCE;c.predicate=(uint8_t)PNP_PRED_EQ;c.compare_source.kind=(uint8_t)PNP_SRC_STATE_WORD;c.predicate_invert=1u;c.update_mode=(uint8_t)PNP_UPDATE_OPERAND_A;e.sequence=5u;s.words[0]=4u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==1u&&next.words[0]==5u);s=next;CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==0u&&next.words[0]==5u);
    c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;c.predicate=(uint8_t)PNP_PRED_LT_U;c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=100u;e.type=PNP_EVENT_TYPE_TIME;e.fields[0]=99u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==1u);e.fields[0]=100u;CHECK(run(&c,&s,&e,&next,&o)==0&&o.count==0u);
    c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_b.kind=(uint8_t)PNP_SRC_ONE;c.alu_op=(uint8_t)PNP_ALU_XOR;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;s.words[0]=0u;CHECK(run(&c,&s,&e,&next,&o)==0&&next.words[0]==1u&&o.items[0].event.fields[0]==1u);
    CHECK(ack_expressivity()==0);CHECK(retry_type_expressivity()==0);return 0;
}
