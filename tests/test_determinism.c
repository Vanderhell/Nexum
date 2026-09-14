#include "test_support.h"

static int replay_shared_graph(void){
    pnp_node_t nodes[2];pnp_state_domain_t domain,final_domain;pnp_edge_t edges[2]={{0u,0u,1u,0u},{1u,0u,PNP_EXTERNAL_NODE,0u}};pnp_graph_t graph={nodes,2u,2u,edges,2u,2u,&domain,1u,1u};pnp_graph_event_t queue_store[8];pnp_queue_t queue;pnp_graph_output_t first_items[4],second_items[4];pnp_graph_output_buffer_t first={first_items,4u,0u},second={second_items,4u,0u};pnp_run_stats_t stats;pnp_event_t events[2];
    memset(nodes,0,sizeof(nodes));memset(&domain,0,sizeof(domain));memset(events,0,sizeof(events));nodes[0].config=pass_config(1u);nodes[0].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;nodes[0].config.source_b.kind=(uint8_t)PNP_SRC_ONE;nodes[0].config.alu_op=(uint8_t)PNP_ALU_ADD;nodes[0].config.update_mode=(uint8_t)PNP_UPDATE_RESULT;nodes[1].config=nodes[0].config;nodes[1].config.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;nodes[1].config.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;nodes[1].config.event_update_constant=UINT32_C(0x41434b00);events[0].sequence=1u;events[1].sequence=2u;
    pnp_queue_init(&queue,queue_store,8u);CHECK(pnp_graph_inject(&queue,0u,0u,&events[0])==PNP_OK);CHECK(pnp_graph_inject(&queue,0u,0u,&events[1])==PNP_OK);CHECK(pnp_graph_run(&graph,&queue,&first,(pnp_run_limits_t){4u,4u},&stats)==PNP_OK);final_domain=domain;
    memset(&domain,0,sizeof(domain));pnp_queue_init(&queue,queue_store,8u);CHECK(pnp_graph_inject(&queue,0u,0u,&events[0])==PNP_OK);CHECK(pnp_graph_inject(&queue,0u,0u,&events[1])==PNP_OK);CHECK(pnp_graph_run(&graph,&queue,&second,(pnp_run_limits_t){4u,4u},&stats)==PNP_OK);CHECK(first.count==second.count);CHECK(memcmp(first.items,second.items,(size_t)first.count*sizeof(first.items[0]))==0);CHECK(memcmp(&final_domain,&domain,sizeof(domain))==0);return 0;
}

int main(void){
    pnp_config_t c=pass_config(UINT32_C(0xff)),before_c;c.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;c.source_a.index=7u;c.source_b.kind=(uint8_t)PNP_SRC_CONSTANT;c.source_b.immediate=1u;c.alu_op=(uint8_t)PNP_ALU_ADD;c.update_mode=(uint8_t)PNP_UPDATE_RESULT;c.update_index=7u;c.event_update_destination=(uint8_t)PNP_EVENT_DEST_METADATA;c.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;c.event_update_index=7u;
    pnp_state_t s={{0}},before_s,reference_state,current_state;pnp_event_t e={0},before_e;e.sequence=UINT64_MAX;e.fields[0]=UINT64_MAX;s.words[7]=UINT64_MAX;before_c=c;before_s=s;before_e=e;
    pnp_output_buffer_t reference,current;CHECK(pnp_execute(&c,&s,&e,&reference_state,&reference)==PNP_OK);
    for(uint32_t i=0u;i<10000u;++i){CHECK(pnp_execute(&c,&s,&e,&current_state,&current)==PNP_OK);CHECK(memcmp(&reference_state,&current_state,sizeof(current_state))==0);CHECK(memcmp(&reference,&current,sizeof(current))==0);}
    CHECK(memcmp(&c,&before_c,sizeof(c))==0);CHECK(memcmp(&s,&before_s,sizeof(s))==0);CHECK(memcmp(&e,&before_e,sizeof(e))==0);CHECK(replay_shared_graph()==0);return 0;
}
