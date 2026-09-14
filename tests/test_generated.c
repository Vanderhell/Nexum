#include <inttypes.h>
#include "test_support.h"

static uint64_t rng_state;
static uint64_t next_random(void){rng_state^=rng_state>>12u;rng_state^=rng_state<<25u;rng_state^=rng_state>>27u;return rng_state*UINT64_C(2685821657736338717);}
static pnp_source_t random_source(void){pnp_source_t s={0};s.kind=(uint8_t)(next_random()%PNP_SRC_COUNT);s.immediate=next_random();if(s.kind==(uint8_t)PNP_SRC_EVENT_FIELD)s.index=(uint8_t)(next_random()%PNP_EVENT_FIELD_COUNT);if(s.kind==(uint8_t)PNP_SRC_STATE_WORD)s.index=(uint8_t)(next_random()%PNP_STATE_WORD_COUNT);return s;}
static int unchanged_except_destination(const pnp_event_t *before,const pnp_event_t *after,uint8_t destination,uint8_t index){pnp_event_t expected=*after;switch((pnp_event_destination_t)destination){case PNP_EVENT_DEST_NONE:break;case PNP_EVENT_DEST_TYPE:expected.type=before->type;break;case PNP_EVENT_DEST_FLAGS:expected.flags=before->flags;break;case PNP_EVENT_DEST_SEQUENCE:expected.sequence=before->sequence;break;case PNP_EVENT_DEST_EPOCH:expected.epoch=before->epoch;break;case PNP_EVENT_DEST_METADATA:expected.fields[index]=before->fields[index];break;default:return 0;}return memcmp(&expected,before,sizeof(expected))==0;}

int main(void){
    const uint64_t seeds[]={UINT64_C(1),UINT64_C(0x4d595df4d0f33173),UINT64_C(0xaaaaaaaa55555555),UINT64_C(0xffffffffffffffff)};
    for(uint32_t seed_index=0u;seed_index<4u;++seed_index){rng_state=seeds[seed_index];for(uint32_t n=0u;n<2500u;++n){
        pnp_config_t c=pass_config((uint32_t)next_random()&UINT32_C(0xff));pnp_state_t state,before_state,out,again;pnp_event_t event,before_event;pnp_output_buffer_t outputs,again_outputs;memset(&state,0,sizeof(state));memset(&event,0,sizeof(event));
        for(uint32_t i=0u;i<PNP_STATE_WORD_COUNT;++i){state.words[i]=next_random();}
        for(uint32_t i=0u;i<PNP_EVENT_FIELD_COUNT;++i){event.fields[i]=next_random();}
        event.type=(uint32_t)next_random();event.sequence=next_random();event.epoch=next_random();event.flags=next_random();event.payload_ref=next_random();event.payload_length=(uint32_t)next_random();event.input_port=(uint32_t)next_random();
        c.source_a=random_source();c.source_b=random_source();c.compare_source=random_source();c.alu_op=(uint8_t)(next_random()%PNP_ALU_COUNT);if((c.alu_op==(uint8_t)PNP_ALU_SHL||c.alu_op==(uint8_t)PNP_ALU_SHR)&&c.source_b.kind==(uint8_t)PNP_SRC_CONSTANT)c.source_b.immediate%=64u;c.predicate=(uint8_t)(next_random()%PNP_PRED_COUNT);c.predicate_invert=(uint8_t)(next_random()&1u);c.predicate_bit=c.predicate==(uint8_t)PNP_PRED_BIT_SET?(uint8_t)(next_random()%64u):0u;
        c.emit_false=(uint32_t)next_random()&UINT32_C(0xff);c.update_mode=(uint8_t)(next_random()%PNP_UPDATE_COUNT);if(c.update_mode!=(uint8_t)PNP_UPDATE_NONE)c.update_index=(uint8_t)(next_random()%PNP_STATE_WORD_COUNT);if(c.update_mode==(uint8_t)PNP_UPDATE_EVENT_FIELD)c.update_event_index=(uint8_t)(next_random()%PNP_EVENT_FIELD_COUNT);c.update_constant=next_random();
        c.event_update_destination=(uint8_t)(next_random()%PNP_EVENT_DEST_COUNT);c.event_update_source=(uint8_t)(next_random()%PNP_EVENT_VALUE_COUNT);if(c.event_update_destination==(uint8_t)PNP_EVENT_DEST_METADATA)c.event_update_index=(uint8_t)(next_random()%PNP_EVENT_FIELD_COUNT);if(c.event_update_source==(uint8_t)PNP_EVENT_VALUE_STATE_WORD)c.event_update_state_index=(uint8_t)(next_random()%PNP_STATE_WORD_COUNT);if(c.event_update_destination==(uint8_t)PNP_EVENT_DEST_NONE){c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;c.event_update_state_index=0u;}c.event_update_constant=next_random();before_state=state;before_event=event;
        if(pnp_validate_config(&c)!=PNP_OK||pnp_execute(&c,&state,&event,&out,&outputs)!=PNP_OK){fprintf(stderr,"seed=%" PRIu64 " case=%u validation/execution\n",seeds[seed_index],n);return 1;}
        for(uint32_t k=0u;k<3u;++k){CHECK(pnp_execute(&c,&state,&event,&again,&again_outputs)==PNP_OK);CHECK(memcmp(&out,&again,sizeof(out))==0&&memcmp(&outputs,&again_outputs,sizeof(outputs))==0);}CHECK(memcmp(&state,&before_state,sizeof(state))==0&&memcmp(&event,&before_event,sizeof(event))==0);CHECK(outputs.count<=PNP_MAX_OUTPUTS);
        for(uint32_t i=0u;i<PNP_STATE_WORD_COUNT;++i){if(i!=c.update_index||c.update_mode==(uint8_t)PNP_UPDATE_NONE)CHECK(out.words[i]==state.words[i]);}
        for(uint32_t i=0u;i<outputs.count;++i){CHECK(unchanged_except_destination(&event,&outputs.items[i].event,c.event_update_destination,c.event_update_index));}
    }}return 0;
}
