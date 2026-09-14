#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H
#include <stdio.h>
#include <string.h>
#include "pnp_graph.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); return 1; } } while (0)
static pnp_config_t pass_config(uint32_t mask) {
    pnp_config_t c; memset(&c,0,sizeof(c)); c.alu_op=(uint8_t)PNP_ALU_PASS_A; c.predicate=(uint8_t)PNP_PRED_ALWAYS; c.emit_true=mask; return c;
}
#endif
