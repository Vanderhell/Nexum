#ifndef NEXUM_COMPILE_H
#define NEXUM_COMPILE_H

#include "nexum_builder.h"
#include "nexum_config.h"

typedef enum nexum_behavior {
    NEXUM_BEHAVIOR_PASS = 0,
    NEXUM_BEHAVIOR_CONDITIONAL_DROP,
    NEXUM_BEHAVIOR_CONDITIONAL_ROUTE,
    NEXUM_BEHAVIOR_REWRITE_METADATA,
    NEXUM_BEHAVIOR_REWRITE_TYPE,
    NEXUM_BEHAVIOR_COUNTER,
    NEXUM_BEHAVIOR_ACK,
    NEXUM_BEHAVIOR_RETRY,
    NEXUM_BEHAVIOR_COUNT
} nexum_behavior_t;

typedef struct nexum_compile_spec {
    uint8_t behavior;
    uint8_t metadata_index;
    uint8_t state_index;
    uint8_t reserved;
    pnp_word_t match_value;
    pnp_word_t rewrite_value;
    uint32_t data_type;
    uint32_t time_type;
    uint32_t ack_type;
    uint32_t retry_type;
    uint32_t failure_type;
    uint32_t retry_limit;
} nexum_compile_spec_t;

typedef struct nexum_compile_requirements {
    nexum_builder_capacities_t capacities;
    uint8_t exact;
    uint8_t reserved[3];
} nexum_compile_requirements_t;

pnp_result_t nexum_compile_requirements(const nexum_compile_spec_t *spec,
                                        nexum_compile_requirements_t *requirements);
pnp_result_t nexum_compile(const nexum_compile_spec_t *spec,
                           nexum_builder_capacities_t capacities,
                           nexum_program_t *program);

#endif
