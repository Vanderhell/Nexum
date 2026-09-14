#include "test_support.h"

static pnp_config_t increment_config(void){pnp_config_t c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_b.kind=(uint8_t)PNP_SRC_ONE;c.alu_op=(uint8_t)PNP_ALU_ADD;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;return c;}

static int shared_domains(void){
    pnp_node_t nodes[2];pnp_state_domain_t domains[2];pnp_edge_t edges[2];pnp_graph_event_t storage[8];pnp_queue_t q;pnp_graph_output_t items[4];pnp_graph_output_buffer_t out={items,4u,0u};pnp_run_stats_t stats;pnp_event_t e={0};
    pnp_graph_t g={nodes,2u,2u,edges,2u,2u,domains,1u,2u};memset(nodes,0,sizeof(nodes));memset(domains,0,sizeof(domains));nodes[0].config=increment_config();nodes[1].config=increment_config();edges[0]=(pnp_edge_t){0u,0u,1u,0u};edges[1]=(pnp_edge_t){1u,0u,PNP_EXTERNAL_NODE,0u};
    pnp_queue_init(&q,storage,8u);CHECK(pnp_graph_inject(&q,0u,0u,&e)==PNP_OK);CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){2u,2u},&stats)==PNP_OK);CHECK(domains[0].state.words[0]==2u);
    domains[0].state.words[0]=0u;edges[0]=(pnp_edge_t){1u,0u,0u,0u};edges[1]=(pnp_edge_t){0u,0u,PNP_EXTERNAL_NODE,0u};out.count=0u;pnp_queue_init(&q,storage,8u);CHECK(pnp_graph_inject(&q,1u,0u,&e)==PNP_OK);CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){2u,2u},&stats)==PNP_OK);CHECK(domains[0].state.words[0]==2u);
    nodes[1].state_domain=1u;g.state_domain_count=2u;domains[0].state.words[0]=0u;domains[1].state.words[0]=0u;edges[0]=(pnp_edge_t){0u,0u,1u,0u};edges[1]=(pnp_edge_t){1u,0u,PNP_EXTERNAL_NODE,0u};out.count=0u;pnp_queue_init(&q,storage,8u);CHECK(pnp_graph_inject(&q,0u,0u,&e)==PNP_OK);CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){2u,2u},&stats)==PNP_OK);CHECK(domains[0].state.words[0]==1u&&domains[1].state.words[0]==1u);
    nodes[1].state_domain=0u;g.state_domain_count=1u;domains[0].state.words[0]=0u;e.sequence=1u;pnp_queue_init(&q,storage,8u);CHECK(pnp_graph_inject(&q,0u,0u,&e)==PNP_OK);e.sequence=2u;CHECK(pnp_graph_inject(&q,0u,0u,&e)==PNP_OK);out.count=0u;CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){4u,4u},&stats)==PNP_OK);CHECK(domains[0].state.words[0]==4u);CHECK(out.count==2u&&out.items[0].event.sequence==1u&&out.items[1].event.sequence==2u);return 0;
}

int main(void){
    pnp_graph_event_t store[8],a={0},b={0},got;pnp_queue_t q;pnp_run_stats_t stats;pnp_event_t input={0};
    pnp_node_t nodes[2];pnp_state_domain_t domains[2];pnp_edge_t edges[3];pnp_graph_t g={nodes,2u,2u,edges,3u,3u,domains,2u,2u};pnp_graph_output_t os[4];pnp_graph_output_buffer_t out={os,4u,0u};memset(nodes,0,sizeof(nodes));memset(domains,0,sizeof(domains));nodes[1].state_domain=1u;
    pnp_queue_init(&q,store,1u);CHECK(pnp_queue_empty(&q));a.node_id=10u;b.node_id=11u;CHECK(pnp_queue_push(&q,&a)==PNP_OK);CHECK(pnp_queue_full(&q));CHECK(pnp_queue_push(&q,&b)==PNP_ERR_QUEUE_FULL);CHECK(pnp_queue_pop(&q,&got)==PNP_OK&&got.node_id==10u);CHECK(pnp_queue_pop(&q,&got)==PNP_ERR_QUEUE_EMPTY);
    nodes[0].config=pass_config(1u);nodes[1].config=pass_config(1u);edges[0]=(pnp_edge_t){0u,0u,1u,7u};edges[1]=(pnp_edge_t){1u,0u,PNP_EXTERNAL_NODE,2u};edges[2]=(pnp_edge_t){1u,0u,PNP_EXTERNAL_NODE,3u};
    CHECK(pnp_graph_validate(&g,8u)==PNP_OK);nodes[1].state_domain=2u;CHECK(pnp_graph_validate(&g,8u)==PNP_ERR_INVALID_GRAPH);nodes[1].state_domain=1u;pnp_queue_init(&q,store,8u);input.sequence=77u;CHECK(pnp_graph_inject(&q,0u,4u,&input)==PNP_OK);CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){2u,3u},&stats)==PNP_OK);CHECK(out.count==2u&&out.items[0].port==2u&&out.items[1].port==3u);
    {pnp_node_t n; pnp_state_domain_t d; pnp_edge_t ee[2];pnp_graph_t cyc={&n,1u,1u,ee,2u,2u,&d,1u,1u};pnp_graph_output_t oo[1];pnp_graph_output_buffer_t ob={oo,1u,0u};memset(&n,0,sizeof(n));memset(&d,0,sizeof(d));n.config=increment_config();n.config.predicate=(uint8_t)PNP_PRED_LT_U;n.config.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;n.config.compare_source.immediate=3u;n.config.emit_false=2u;ee[0]=(pnp_edge_t){0u,0u,0u,0u};ee[1]=(pnp_edge_t){0u,1u,PNP_EXTERNAL_NODE,0u};pnp_queue_init(&q,store,8u);CHECK(pnp_graph_inject(&q,0u,0u,&input)==PNP_OK);CHECK(pnp_graph_run(&cyc,&q,&ob,(pnp_run_limits_t){3u,3u},&stats)==PNP_OK);CHECK(d.state.words[0]==2u&&ob.count==1u);}
    CHECK(shared_domains()==0);return 0;
}
