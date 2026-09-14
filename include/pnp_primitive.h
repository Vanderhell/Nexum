#ifndef PNP_PRIMITIVE_H
#define PNP_PRIMITIVE_H

#include "pnp_result.h"
#include "pnp_types.h"

typedef enum pnp_source_kind {
    PNP_SRC_ZERO = 0, PNP_SRC_ONE, PNP_SRC_CONSTANT, PNP_SRC_EVENT_FIELD,
    PNP_SRC_STATE_WORD, PNP_SRC_TYPE, PNP_SRC_SEQUENCE, PNP_SRC_EPOCH,
    PNP_SRC_FLAGS, PNP_SRC_INPUT_PORT, PNP_SRC_COUNT
} pnp_source_kind_t;

typedef struct pnp_source {
    uint8_t kind;
    uint8_t index;
    uint16_t reserved;
    pnp_word_t immediate;
} pnp_source_t;

typedef enum pnp_alu_op {
    PNP_ALU_PASS_A = 0, PNP_ALU_ADD, PNP_ALU_SUB, PNP_ALU_AND,
    PNP_ALU_OR, PNP_ALU_XOR, PNP_ALU_SHL, PNP_ALU_SHR, PNP_ALU_COUNT
} pnp_alu_op_t;

typedef enum pnp_predicate {
    PNP_PRED_ALWAYS = 0, PNP_PRED_EQ, PNP_PRED_LT_U, PNP_PRED_BIT_SET,
    PNP_PRED_COUNT
} pnp_predicate_t;

typedef enum pnp_update_mode {
    PNP_UPDATE_NONE = 0, PNP_UPDATE_RESULT, PNP_UPDATE_OPERAND_A,
    PNP_UPDATE_OPERAND_B, PNP_UPDATE_CONSTANT, PNP_UPDATE_EVENT_FIELD,
    PNP_UPDATE_COUNT
} pnp_update_mode_t;

typedef enum pnp_event_destination {
    PNP_EVENT_DEST_NONE = 0, PNP_EVENT_DEST_TYPE, PNP_EVENT_DEST_FLAGS,
    PNP_EVENT_DEST_SEQUENCE, PNP_EVENT_DEST_EPOCH, PNP_EVENT_DEST_METADATA,
    PNP_EVENT_DEST_COUNT
} pnp_event_destination_t;

typedef enum pnp_event_update_source {
    PNP_EVENT_VALUE_RESULT = 0, PNP_EVENT_VALUE_OPERAND_A,
    PNP_EVENT_VALUE_OPERAND_B, PNP_EVENT_VALUE_STATE_WORD,
    PNP_EVENT_VALUE_CONSTANT, PNP_EVENT_VALUE_COUNT
} pnp_event_update_source_t;

typedef struct pnp_config {
    pnp_source_t source_a;
    pnp_source_t source_b;
    pnp_source_t compare_source;
    pnp_word_t update_constant;
    pnp_word_t event_update_constant;
    uint32_t emit_true;
    uint32_t emit_false;
    uint8_t alu_op;
    uint8_t predicate;
    uint8_t predicate_invert;
    uint8_t predicate_bit;
    uint8_t update_mode;
    uint8_t update_index;
    uint8_t update_event_index;
    uint8_t event_update_destination;
    uint8_t event_update_source;
    uint8_t event_update_index;
    uint8_t event_update_state_index;
    uint16_t reserved;
} pnp_config_t;

pnp_result_t pnp_validate_config(const pnp_config_t *config);
pnp_result_t pnp_execute(const pnp_config_t *config, const pnp_state_t *state_in,
                         const pnp_event_t *event_in, pnp_state_t *state_out,
                         pnp_output_buffer_t *outputs);

#endif
