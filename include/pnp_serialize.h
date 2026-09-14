#ifndef PNP_SERIALIZE_H
#define PNP_SERIALIZE_H
#include <stddef.h>
#include "pnp_primitive.h"
#define PNP_CONFIG_WIRE_SIZE 81u
#define PNP_STATE_WIRE_SIZE (8u + PNP_STATE_WORD_COUNT * 8u)
pnp_result_t pnp_config_encode_v0(const pnp_config_t *value, uint8_t *data, size_t size);
pnp_result_t pnp_config_decode_v0(const uint8_t *data, size_t size, pnp_config_t *value);
pnp_result_t pnp_state_encode_v0(const pnp_state_t *value, uint8_t *data, size_t size);
pnp_result_t pnp_state_decode_v0(const uint8_t *data, size_t size, pnp_state_t *value);
#endif
