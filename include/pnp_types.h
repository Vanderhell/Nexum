#ifndef PNP_TYPES_H
#define PNP_TYPES_H

#include <stdint.h>

/* Experimental implementation limits; these are not a stable ABI. */
#define PNP_EVENT_FIELD_COUNT 8u
#define PNP_STATE_WORD_COUNT 8u
#define PNP_MAX_OUTPUTS 8u
#define PNP_GRAPH_INPUT_PORT_COUNT 256u

#define PNP_EVENT_TYPE_TIME UINT32_C(0x54494d45)

typedef uint64_t pnp_word_t;

typedef struct pnp_event {
    uint32_t type;
    uint32_t input_port;
    uint64_t sequence;
    uint64_t epoch;
    uint64_t flags;
    uint64_t payload_ref;
    uint32_t payload_length;
    uint32_t reserved;
    pnp_word_t fields[PNP_EVENT_FIELD_COUNT];
} pnp_event_t;

typedef struct pnp_state { pnp_word_t words[PNP_STATE_WORD_COUNT]; } pnp_state_t;
typedef struct pnp_output { uint32_t port; uint32_t reserved; pnp_event_t event; } pnp_output_t;
typedef struct pnp_output_buffer { uint32_t count; uint32_t reserved; pnp_output_t items[PNP_MAX_OUTPUTS]; } pnp_output_buffer_t;

_Static_assert(sizeof(pnp_word_t) == 8u, "pnp_word_t must be 64-bit");
_Static_assert(PNP_MAX_OUTPUTS <= 32u, "output masks use uint32_t");

#endif
