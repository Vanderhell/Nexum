#include "test_support.h"
#include "pnp_serialize.h"
int main(void){
    pnp_config_t c=pass_config(3u),d;uint8_t cb[PNP_CONFIG_WIRE_SIZE];
    pnp_state_t s={{1u,2u,3u,4u,5u,6u,7u,UINT64_MAX}},t;uint8_t sb[PNP_STATE_WIRE_SIZE];
    c.source_a.kind=(uint8_t)PNP_SRC_CONSTANT;c.source_a.immediate=UINT64_MAX;
    c.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;c.event_update_constant=42u;
    CHECK(pnp_config_encode_v0(&c,cb,sizeof(cb))==PNP_OK);CHECK(pnp_config_decode_v0(cb,sizeof(cb),&d)==PNP_OK&&memcmp(&c,&d,sizeof(c))==0);
    cb[0]=0u;CHECK(pnp_config_decode_v0(cb,sizeof(cb),&d)==PNP_ERR_BAD_FORMAT);
    CHECK(pnp_state_encode_v0(&s,sb,sizeof(sb))==PNP_OK);CHECK(pnp_state_decode_v0(sb,sizeof(sb),&t)==PNP_OK&&memcmp(&s,&t,sizeof(s))==0);
    CHECK(pnp_state_decode_v0(sb,sizeof(sb)-1u,&t)==PNP_ERR_BAD_FORMAT);return 0;
}
