#include <string.h>
#include "pnp_serialize.h"

static void put16(uint8_t *p, uint16_t v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8u); }
static void put32(uint8_t *p, uint32_t v) { unsigned i; for(i=0u;i<4u;++i)p[i]=(uint8_t)(v>>(i*8u)); }
static void put64(uint8_t *p, uint64_t v) { unsigned i; for(i=0u;i<8u;++i)p[i]=(uint8_t)(v>>(i*8u)); }
static uint16_t get16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1]<<8u)); }
static uint32_t get32(const uint8_t *p) { uint32_t v=0u; unsigned i; for(i=0u;i<4u;++i)v|=(uint32_t)p[i]<<(i*8u); return v; }
static uint64_t get64(const uint8_t *p) { uint64_t v=0u; unsigned i; for(i=0u;i<8u;++i)v|=(uint64_t)p[i]<<(i*8u); return v; }
static void source_put(uint8_t *p,const pnp_source_t *s){p[0]=s->kind;p[1]=s->index;put16(p+2,s->reserved);put64(p+4,s->immediate);}
static void source_get(const uint8_t *p,pnp_source_t *s){s->kind=p[0];s->index=p[1];s->reserved=get16(p+2);s->immediate=get64(p+4);}
static int header(const uint8_t *d,size_t n,uint8_t kind){return n>=8u&&d[0]=='P'&&d[1]=='N'&&d[2]=='P'&&d[3]==kind&&d[4]==0u&&d[5]==0u&&d[6]==0u&&d[7]==0u;}
pnp_result_t pnp_config_encode_v0(const pnp_config_t *v,uint8_t *d,size_t n){
    size_t o=8u; if(v==NULL||d==NULL)return PNP_ERR_INVALID_ARGUMENT;if(n<PNP_CONFIG_WIRE_SIZE)return PNP_ERR_BUFFER_FULL;if(pnp_validate_config(v)!=PNP_OK)return PNP_ERR_INVALID_CONFIG;
    memset(d,0,PNP_CONFIG_WIRE_SIZE);d[0]='P';d[1]='N';d[2]='P';d[3]='C';
    source_put(d+o,&v->source_a);o+=12u;source_put(d+o,&v->source_b);o+=12u;source_put(d+o,&v->compare_source);o+=12u;
    put64(d+o,v->update_constant);o+=8u;put64(d+o,v->event_update_constant);o+=8u;put32(d+o,v->emit_true);o+=4u;put32(d+o,v->emit_false);o+=4u;
    d[o++]=v->alu_op;d[o++]=v->predicate;d[o++]=v->predicate_invert;d[o++]=v->predicate_bit;d[o++]=v->update_mode;d[o++]=v->update_index;d[o++]=v->update_event_index;d[o++]=v->event_update_destination;d[o++]=v->event_update_source;d[o++]=v->event_update_index;d[o++]=v->event_update_state_index;put16(d+o,v->reserved);return PNP_OK;
}
pnp_result_t pnp_config_decode_v0(const uint8_t *d,size_t n,pnp_config_t *v){
    size_t o=8u;if(d==NULL||v==NULL)return PNP_ERR_INVALID_ARGUMENT;if(n!=PNP_CONFIG_WIRE_SIZE||!header(d,n,'C'))return PNP_ERR_BAD_FORMAT;memset(v,0,sizeof(*v));
    source_get(d+o,&v->source_a);o+=12u;source_get(d+o,&v->source_b);o+=12u;source_get(d+o,&v->compare_source);o+=12u;
    v->update_constant=get64(d+o);o+=8u;v->event_update_constant=get64(d+o);o+=8u;v->emit_true=get32(d+o);o+=4u;v->emit_false=get32(d+o);o+=4u;
    v->alu_op=d[o++];v->predicate=d[o++];v->predicate_invert=d[o++];v->predicate_bit=d[o++];v->update_mode=d[o++];v->update_index=d[o++];v->update_event_index=d[o++];v->event_update_destination=d[o++];v->event_update_source=d[o++];v->event_update_index=d[o++];v->event_update_state_index=d[o++];v->reserved=get16(d+o);
    return pnp_validate_config(v)==PNP_OK?PNP_OK:PNP_ERR_BAD_FORMAT;
}
pnp_result_t pnp_state_encode_v0(const pnp_state_t *v,uint8_t *d,size_t n){uint32_t i;if(v==NULL||d==NULL)return PNP_ERR_INVALID_ARGUMENT;if(n<PNP_STATE_WIRE_SIZE)return PNP_ERR_BUFFER_FULL;memset(d,0,PNP_STATE_WIRE_SIZE);d[0]='P';d[1]='N';d[2]='P';d[3]='S';for(i=0u;i<PNP_STATE_WORD_COUNT;++i)put64(d+8u+(size_t)i*8u,v->words[i]);return PNP_OK;}
pnp_result_t pnp_state_decode_v0(const uint8_t *d,size_t n,pnp_state_t *v){uint32_t i;if(d==NULL||v==NULL)return PNP_ERR_INVALID_ARGUMENT;if(n!=PNP_STATE_WIRE_SIZE||!header(d,n,'S'))return PNP_ERR_BAD_FORMAT;for(i=0u;i<PNP_STATE_WORD_COUNT;++i)v->words[i]=get64(d+8u+(size_t)i*8u);return PNP_OK;}
