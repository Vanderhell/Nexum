#include "test_support.h"
#include "nexum_program.h"

typedef struct lowered_fixture {
    pnp_node_t nodes[16];
    pnp_edge_t edges[32];
    pnp_state_domain_t domains[8];
    pnp_graph_t graph;
} lowered_fixture_t;

static void minimal(nexum_program_t *p) {
    memset(p, 0, sizeof(*p));
}

static void one_cell(nexum_program_t *p, pnp_config_t config) {
    minimal(p); p->cell_count = 1u; p->state_domain_count = 1u;
    p->cells[0] = (nexum_cell_t){10u, 7u, config};
    p->state_domains[0].id = 7u;
    p->input_count = 1u; p->inputs[0] = (nexum_input_t){3u, 10u, 0u, 0u};
    p->output_count = 1u; p->outputs[0] = (nexum_output_t){5u, 10u, 0u, 0u};
}

static pnp_result_t lower(nexum_program_t *p, lowered_fixture_t *f) {
    nexum_runtime_storage_t s;
    memset(f, 0, sizeof(*f));
    s = (nexum_runtime_storage_t){f->nodes, 16u, f->edges, 32u, f->domains, 8u};
    return nexum_program_lower(p, &s, &f->graph);
}

static int validation_tests(void) {
    nexum_program_t p; nexum_program_requirements_t r; lowered_fixture_t f;
    minimal(&p); CHECK(nexum_program_validate(&p) == PNP_OK);
    CHECK(nexum_program_requirements(&p, &r) == PNP_OK && r.runtime_storage_bytes == 0u);
    CHECK(lower(&p, &f) == PNP_OK && f.graph.node_count == 0u);
    one_cell(&p, pass_config(1u)); CHECK(nexum_program_validate(&p) == PNP_OK);
    CHECK(nexum_program_requirements(&p, &r) == PNP_OK && r.node_count == 1u && r.edge_count == 1u && r.state_domain_count == 1u);
    p.cells[0].id = UINT32_MAX; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.cells[0].id = 10u;
    p.cells[0].state_domain_id = 8u; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.cells[0].state_domain_id = 7u;
    p.cells[0].config.alu_op = (uint8_t)PNP_ALU_COUNT; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.cells[0].config = pass_config(1u);
    p.inputs[0].destination_cell_id = 11u; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.inputs[0].destination_cell_id = 10u;
    p.outputs[0].source_port = PNP_MAX_OUTPUTS; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.outputs[0].source_port = 0u;
    p.edge_count = 1u; p.edges[0] = (nexum_edge_t){10u, 0u, 11u, 0u, 1u, 0u};
    CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.edge_count = 0u;
    p.input_count = 2u; p.inputs[1] = p.inputs[0]; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.input_count = 1u;
    p.output_count = 2u; p.outputs[1] = p.outputs[0]; CHECK(nexum_program_validate(&p) == PNP_ERR_INVALID_GRAPH); p.output_count = 1u;
    p.cell_count = 2u; p.cells[1] = p.cells[0]; p.cells[1].id = 20u;
    CHECK(nexum_program_validate(&p) == PNP_OK); /* shared domain */
    p.state_domain_count = 2u; p.state_domains[1].id = 9u; p.cells[1].state_domain_id = 9u;
    CHECK(nexum_program_validate(&p) == PNP_OK); /* isolated domains */
    { nexum_runtime_storage_t small = {f.nodes, 1u, f.edges, 0u, f.domains, 2u};
      CHECK(nexum_program_lower(&p, &small, &f.graph) == PNP_ERR_BUFFER_FULL); }
    return 0;
}

static void program_from_graph(nexum_program_t *p, const pnp_graph_t *g) {
    uint32_t i, internal = 0u, outputs = 0u;
    minimal(p); p->cell_count = g->node_count; p->state_domain_count = g->state_domain_count;
    for (i = 0u; i < g->node_count; ++i) {
        uint32_t source = g->node_count - 1u - i;
        p->cells[i] = (nexum_cell_t){100u + source, 500u + g->nodes[source].state_domain, g->nodes[source].config};
    }
    for (i = 0u; i < g->state_domain_count; ++i) { p->state_domains[i].id = 500u + i; p->state_domains[i].initial_state = g->state_domains[i].state; }
    for (i = 0u; i < g->edge_count; ++i) {
        const pnp_edge_t *e = &g->edges[i]; uint32_t order = 0u, j;
        for (j = 0u; j < i; ++j) if (g->edges[j].source_node == e->source_node && g->edges[j].source_port == e->source_port) ++order;
        if (e->destination_node == PNP_EXTERNAL_NODE)
            p->outputs[outputs++] = (nexum_output_t){e->destination_port, 100u + e->source_node, e->source_port, order};
        else p->edges[internal++] = (nexum_edge_t){100u + e->source_node, e->source_port, 100u + e->destination_node, e->destination_port, order, 0u};
    }
    p->edge_count = internal; p->output_count = outputs; p->input_count = 1u;
    p->inputs[0] = (nexum_input_t){77u, 100u, 0u, 0u};
}

static int equivalent(pnp_graph_t *direct, const pnp_event_t *events, uint32_t count) {
    nexum_program_t p; lowered_fixture_t lf; pnp_graph_event_t qa_items[64], qb_items[64]; pnp_queue_t qa, qb;
    pnp_graph_output_t oa_items[64], ob_items[64]; pnp_graph_output_buffer_t oa={oa_items,64u,0u},ob={ob_items,64u,0u};
    pnp_run_stats_t sa={0}, sb={0}; pnp_state_domain_t initial[8]; uint32_t i;
    CHECK(direct->state_domain_count <= 8u); memcpy(initial,direct->state_domains,(size_t)direct->state_domain_count*sizeof(initial[0]));
    program_from_graph(&p,direct); CHECK(lower(&p,&lf)==PNP_OK); CHECK(pnp_graph_validate(&lf.graph,64u)==PNP_OK);
    pnp_queue_init(&qa,qa_items,64u); pnp_queue_init(&qb,qb_items,64u);
    for(i=0u;i<count;++i){CHECK(pnp_graph_inject(&qa,0u,0u,&events[i])==PNP_OK);CHECK(nexum_program_inject(&p,&qb,77u,&events[i])==PNP_OK);}
    CHECK(pnp_graph_run(direct,&qa,&oa,(pnp_run_limits_t){256u,256u},&sa)==PNP_OK);
    CHECK(pnp_graph_run(&lf.graph,&qb,&ob,(pnp_run_limits_t){256u,256u},&sb)==PNP_OK);
    CHECK(oa.count==ob.count && memcmp(oa.items,ob.items,(size_t)oa.count*sizeof(oa.items[0]))==0);
    CHECK(sa.steps==sb.steps&&sa.emitted_events==sb.emitted_events&&sa.queue_high_water==sb.queue_high_water);
    CHECK(memcmp(direct->state_domains,lf.graph.state_domains,(size_t)direct->state_domain_count*sizeof(initial[0]))==0);
    memcpy(direct->state_domains,initial,(size_t)direct->state_domain_count*sizeof(initial[0])); return 0;
}

static int equivalence_tests(void) {
    pnp_node_t n[3]; pnp_edge_t e[6]; pnp_state_domain_t d[3]; pnp_graph_t g;
    pnp_event_t inputs[3]={{0}}; memset(n,0,sizeof(n)); memset(d,0,sizeof(d));
    /* pass and ACK mutation */
    n[0].config=pass_config(1u); e[0]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,4u}; g=(pnp_graph_t){n,1u,1u,e,1u,1u,d,1u,1u}; CHECK(equivalent(&g,inputs,1u)==0);
    n[0].config.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;n[0].config.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;n[0].config.event_update_constant=UINT32_C(0x41434b00);CHECK(equivalent(&g,inputs,1u)==0);
    /* conditional route and fan-out */
    n[0].config=pass_config(1u);n[0].config.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;n[0].config.predicate=(uint8_t)PNP_PRED_EQ;n[0].config.compare_source.kind=(uint8_t)PNP_SRC_ONE;n[0].config.emit_false=2u;
    e[0]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,1u};e[1]=(pnp_edge_t){0u,1u,PNP_EXTERNAL_NODE,2u};g.edge_count=2u;g.edge_capacity=2u;inputs[0].fields[0]=1u;inputs[1].fields[0]=0u;CHECK(equivalent(&g,inputs,2u)==0);
    n[0].config=pass_config(1u);e[0]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,2u};e[1]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,1u};CHECK(equivalent(&g,inputs,1u)==0);
    /* counter, round-robin, failover-style branch, and D2 replay stream */
    n[0].config=pass_config(1u);n[0].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;n[0].config.source_b.kind=(uint8_t)PNP_SRC_ONE;n[0].config.alu_op=(uint8_t)PNP_ALU_ADD;n[0].config.update_mode=(uint8_t)PNP_UPDATE_RESULT;e[0]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,0u};g.edge_count=1u;g.edge_capacity=1u;CHECK(equivalent(&g,inputs,3u)==0);
    n[0].config.predicate=(uint8_t)PNP_PRED_BIT_SET;n[0].config.predicate_bit=0u;n[0].config.emit_false=2u;e[1]=(pnp_edge_t){0u,1u,PNP_EXTERNAL_NODE,1u};g.edge_count=2u;g.edge_capacity=2u;CHECK(equivalent(&g,inputs,3u)==0);
    n[0].config=pass_config(1u);n[0].config.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;n[0].config.predicate=(uint8_t)PNP_PRED_EQ;n[0].config.compare_source.kind=(uint8_t)PNP_SRC_ZERO;n[0].config.emit_false=2u;CHECK(equivalent(&g,inputs,2u)==0);
    /* retry loop: count ticks, emit retry twice, then failure. */
    memset(d,0,sizeof(d));n[0].config=pass_config(1u);n[0].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;n[0].config.source_b.kind=(uint8_t)PNP_SRC_ONE;n[0].config.alu_op=(uint8_t)PNP_ALU_ADD;n[0].config.predicate=(uint8_t)PNP_PRED_LT_U;n[0].config.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;n[0].config.compare_source.immediate=3u;n[0].config.update_mode=(uint8_t)PNP_UPDATE_RESULT;n[0].config.emit_false=2u;e[0]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,6u};e[1]=(pnp_edge_t){0u,1u,PNP_EXTERNAL_NODE,7u};CHECK(equivalent(&g,inputs,3u)==0);
    return 0;
}

static int deterministic_lowering(void) {
    nexum_program_t a,b; lowered_fixture_t x,y; one_cell(&a,pass_config(1u)); b=a;
    a.cell_count=b.cell_count=2u;a.state_domain_count=b.state_domain_count=2u;
    a.cells[1]=(nexum_cell_t){2u,1u,pass_config(2u)};a.state_domains[1].id=1u;b=a;
    b.cells[0]=a.cells[1];b.cells[1]=a.cells[0];b.state_domains[0]=a.state_domains[1];b.state_domains[1]=a.state_domains[0];
    CHECK(lower(&a,&x)==PNP_OK&&lower(&b,&y)==PNP_OK);
    CHECK(memcmp(x.nodes,y.nodes,2u*sizeof(x.nodes[0]))==0);CHECK(memcmp(x.domains,y.domains,2u*sizeof(x.domains[0]))==0);CHECK(memcmp(x.edges,y.edges,sizeof(x.edges[0]))==0);return 0;
}

int main(void) { CHECK(validation_tests()==0); CHECK(deterministic_lowering()==0); CHECK(equivalence_tests()==0); return 0; }
