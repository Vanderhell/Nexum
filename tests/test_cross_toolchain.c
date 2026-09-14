#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "pnp_serialize.h"

int main(void){
    pnp_config_t c; pnp_state_t state={{0}},next; pnp_event_t event={0}; pnp_output_buffer_t outputs; uint8_t config_bytes[PNP_CONFIG_WIRE_SIZE],state_bytes[PNP_STATE_WIRE_SIZE];
    memset(&c,0,sizeof(c));c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_a.index=7u;c.source_b.kind=(uint8_t)PNP_SRC_CONSTANT;c.source_b.immediate=UINT64_C(0x8000000000000001);c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=UINT64_C(0x8000000000000000);c.alu_op=(uint8_t)PNP_ALU_ADD;c.predicate=(uint8_t)PNP_PRED_LT_U;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.update_index=7u;c.emit_true=UINT32_C(0x81);c.emit_false=UINT32_C(0x02);c.event_update_destination=(uint8_t)PNP_EVENT_DEST_FLAGS;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;
    state.words[0]=UINT64_MAX;state.words[7]=UINT64_MAX;event.type=UINT32_C(0xfedcba98);event.input_port=255u;event.sequence=UINT64_MAX;event.epoch=UINT64_C(0x8000000000000000);event.flags=UINT64_C(0xaaaaaaaaaaaaaaaa);event.payload_ref=UINT64_C(0x1122334455667788);event.payload_length=UINT32_MAX;for(uint32_t i=0u;i<PNP_EVENT_FIELD_COUNT;++i)event.fields[i]=UINT64_C(0x0101010101010101)*(uint64_t)(i+1u);
    if(pnp_execute(&c,&state,&event,&next,&outputs)!=PNP_OK||pnp_config_encode_v0(&c,config_bytes,sizeof(config_bytes))!=PNP_OK||pnp_state_encode_v0(&next,state_bytes,sizeof(state_bytes))!=PNP_OK)return 1;
    printf("status=0 count=%u\n",outputs.count);for(uint32_t i=0u;i<PNP_STATE_WIRE_SIZE;++i)printf("%02x",(unsigned)state_bytes[i]);putchar('\n');for(uint32_t i=0u;i<PNP_CONFIG_WIRE_SIZE;++i)printf("%02x",(unsigned)config_bytes[i]);putchar('\n');
    for(uint32_t i=0u;i<outputs.count;++i){const pnp_event_t *e=&outputs.items[i].event;printf("port=%u type=%08" PRIx32 " input=%08" PRIx32 " seq=%016" PRIx64 " epoch=%016" PRIx64 " flags=%016" PRIx64 " payload=%016" PRIx64 " length=%08" PRIx32,outputs.items[i].port,e->type,e->input_port,e->sequence,e->epoch,e->flags,e->payload_ref,e->payload_length);for(uint32_t j=0u;j<PNP_EVENT_FIELD_COUNT;++j)printf(" f%u=%016" PRIx64,j,e->fields[j]);putchar('\n');}
    return 0;
}
