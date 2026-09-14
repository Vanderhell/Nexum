#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "pnp_primitive.h"
#include "pnp_serialize.h"
int LLVMFuzzerTestOneInput(const uint8_t *data,size_t size){pnp_config_t c;pnp_state_t s={{0}},out;pnp_event_t e={0};pnp_output_buffer_t outputs;if(size>=sizeof(c)){memcpy(&c,data,sizeof(c));if(pnp_validate_config(&c)==PNP_OK)(void)pnp_execute(&c,&s,&e,&out,&outputs);}if(size==PNP_CONFIG_WIRE_SIZE)(void)pnp_config_decode_v0(data,size,&c);if(size==PNP_STATE_WIRE_SIZE)(void)pnp_state_decode_v0(data,size,&s);return 0;}
