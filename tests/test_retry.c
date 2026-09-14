#include "test_support.h"

#define EVENT_DATA UINT32_C(1)
#define EVENT_TIME UINT32_C(2)
#define EVENT_ACK UINT32_C(3)
#define EVENT_RETRY UINT32_C(4)
#define EVENT_FAILURE UINT32_C(5)

typedef struct retry_fixture {
    pnp_node_t nodes[11];pnp_state_domain_t domain;pnp_edge_t edges[13];pnp_graph_t graph;
    pnp_graph_event_t queue_items[16];pnp_queue_t queue;pnp_graph_output_t output_items[16];pnp_graph_output_buffer_t outputs;
} retry_fixture_t;

static pnp_config_t type_match(uint32_t type){pnp_config_t c=pass_config(1u);c.source_a.kind=(uint8_t)PNP_SRC_TYPE;c.predicate=(uint8_t)PNP_PRED_EQ;c.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;c.compare_source.immediate=type;c.emit_false=2u;return c;}
static pnp_config_t constant_write(uint8_t index,pnp_word_t value,uint32_t emit){pnp_config_t c=pass_config(emit);c.update_mode=(uint8_t)PNP_UPDATE_CONSTANT;c.update_index=index;c.update_constant=value;return c;}

static void fixture_init(retry_fixture_t *f){
    memset(f,0,sizeof(*f));f->graph=(pnp_graph_t){f->nodes,11u,11u,f->edges,13u,13u,&f->domain,1u,1u};f->outputs=(pnp_graph_output_buffer_t){f->output_items,16u,0u};
    f->nodes[0].config=type_match(EVENT_DATA);
    f->nodes[1].config=pass_config(1u);f->nodes[1].config.source_a.kind=(uint8_t)PNP_SRC_SEQUENCE;f->nodes[1].config.update_mode=(uint8_t)PNP_UPDATE_RESULT;f->nodes[1].config.update_index=0u;
    f->nodes[2].config=constant_write(1u,1u,1u);f->nodes[3].config=constant_write(2u,0u,1u);
    f->nodes[4].config=type_match(EVENT_TIME);
    f->nodes[5].config=pass_config(1u);f->nodes[5].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;f->nodes[5].config.source_a.index=1u;f->nodes[5].config.predicate=(uint8_t)PNP_PRED_EQ;f->nodes[5].config.compare_source.kind=(uint8_t)PNP_SRC_ONE;
    f->nodes[6].config=pass_config(1u);f->nodes[6].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;f->nodes[6].config.source_a.index=0u;f->nodes[6].config.event_update_destination=(uint8_t)PNP_EVENT_DEST_SEQUENCE;f->nodes[6].config.event_update_source=(uint8_t)PNP_EVENT_VALUE_RESULT;
    f->nodes[7].config=pass_config(1u);f->nodes[7].config.source_a.kind=(uint8_t)PNP_SRC_STATE_WORD;f->nodes[7].config.source_a.index=2u;f->nodes[7].config.source_b.kind=(uint8_t)PNP_SRC_ONE;f->nodes[7].config.alu_op=(uint8_t)PNP_ALU_ADD;f->nodes[7].config.predicate=(uint8_t)PNP_PRED_LT_U;f->nodes[7].config.compare_source.kind=(uint8_t)PNP_SRC_CONSTANT;f->nodes[7].config.compare_source.immediate=4u;f->nodes[7].config.update_mode=(uint8_t)PNP_UPDATE_RESULT;f->nodes[7].config.update_index=2u;f->nodes[7].config.emit_false=2u;f->nodes[7].config.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;f->nodes[7].config.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;f->nodes[7].config.event_update_constant=EVENT_RETRY;
    f->nodes[8].config=type_match(EVENT_ACK);f->nodes[8].config.emit_false=0u;
    f->nodes[9].config=pass_config(1u);f->nodes[9].config.event_update_destination=(uint8_t)PNP_EVENT_DEST_TYPE;f->nodes[9].config.event_update_source=(uint8_t)PNP_EVENT_VALUE_CONSTANT;f->nodes[9].config.event_update_constant=EVENT_FAILURE;
    f->nodes[10].config=constant_write(1u,0u,0u);
    f->edges[0]=(pnp_edge_t){0u,0u,1u,0u};f->edges[1]=(pnp_edge_t){0u,1u,4u,0u};f->edges[2]=(pnp_edge_t){1u,0u,2u,0u};f->edges[3]=(pnp_edge_t){2u,0u,3u,0u};f->edges[4]=(pnp_edge_t){3u,0u,PNP_EXTERNAL_NODE,0u};f->edges[5]=(pnp_edge_t){4u,0u,5u,0u};f->edges[6]=(pnp_edge_t){4u,1u,8u,0u};f->edges[7]=(pnp_edge_t){5u,0u,6u,0u};f->edges[8]=(pnp_edge_t){6u,0u,7u,0u};f->edges[9]=(pnp_edge_t){7u,0u,PNP_EXTERNAL_NODE,1u};f->edges[10]=(pnp_edge_t){7u,1u,9u,0u};f->edges[11]=(pnp_edge_t){8u,0u,10u,0u};f->edges[12]=(pnp_edge_t){9u,0u,PNP_EXTERNAL_NODE,2u};
}

static int inject_and_run(retry_fixture_t *f,uint32_t type,uint64_t sequence){pnp_event_t e={0};pnp_run_stats_t stats;e.type=type;e.sequence=sequence;pnp_queue_init(&f->queue,f->queue_items,16u);CHECK(pnp_graph_inject(&f->queue,0u,0u,&e)==PNP_OK);CHECK(pnp_graph_run(&f->graph,&f->queue,&f->outputs,(pnp_run_limits_t){16u,16u},&stats)==PNP_OK);return 0;}

static int retry_limit(void){retry_fixture_t f;fixture_init(&f);CHECK(inject_and_run(&f,EVENT_DATA,77u)==0);CHECK(f.domain.state.words[0]==77u&&f.domain.state.words[1]==1u&&f.domain.state.words[2]==0u);for(uint32_t i=1u;i<=3u;++i){CHECK(inject_and_run(&f,EVENT_TIME,0u)==0);CHECK(f.outputs.items[i].event.type==EVENT_RETRY&&f.outputs.items[i].event.sequence==77u&&f.domain.state.words[2]==i);}CHECK(inject_and_run(&f,EVENT_TIME,0u)==0);CHECK(f.outputs.count==5u&&f.outputs.items[4].event.type==EVENT_FAILURE&&f.outputs.items[4].event.sequence==77u&&f.outputs.items[4].port==2u);return 0;}
static int ack_before_timeout(void){retry_fixture_t f;fixture_init(&f);CHECK(inject_and_run(&f,EVENT_DATA,88u)==0);CHECK(inject_and_run(&f,EVENT_ACK,88u)==0);CHECK(f.domain.state.words[1]==0u);CHECK(inject_and_run(&f,EVENT_TIME,0u)==0);CHECK(f.outputs.count==1u);return 0;}
static int time_before_ack(void){retry_fixture_t f;fixture_init(&f);CHECK(inject_and_run(&f,EVENT_DATA,99u)==0);CHECK(inject_and_run(&f,EVENT_TIME,0u)==0);CHECK(f.outputs.count==2u&&f.outputs.items[1].event.type==EVENT_RETRY);CHECK(inject_and_run(&f,EVENT_ACK,99u)==0);CHECK(inject_and_run(&f,EVENT_TIME,0u)==0);CHECK(f.outputs.count==2u&&f.domain.state.words[1]==0u);return 0;}
static int replay(void){retry_fixture_t a,b;fixture_init(&a);fixture_init(&b);const uint32_t types[]={EVENT_DATA,EVENT_TIME,EVENT_ACK,EVENT_TIME,EVENT_DATA,EVENT_TIME,EVENT_TIME,EVENT_TIME,EVENT_TIME};const uint64_t seq[]={7u,0u,7u,0u,9u,0u,0u,0u,0u};for(uint32_t i=0u;i<9u;++i){CHECK(inject_and_run(&a,types[i],seq[i])==0);CHECK(inject_and_run(&b,types[i],seq[i])==0);}CHECK(memcmp(&a.domain,&b.domain,sizeof(a.domain))==0);CHECK(a.outputs.count==b.outputs.count);CHECK(memcmp(a.outputs.items,b.outputs.items,(size_t)a.outputs.count*sizeof(a.outputs.items[0]))==0);return 0;}
static int permutations(void){const uint32_t orders[4][3]={{EVENT_DATA,EVENT_TIME,EVENT_ACK},{EVENT_DATA,EVENT_ACK,EVENT_TIME},{EVENT_TIME,EVENT_DATA,EVENT_ACK},{EVENT_ACK,EVENT_DATA,EVENT_TIME}};uint32_t counts[4];for(uint32_t order=0u;order<4u;++order){retry_fixture_t a,b;fixture_init(&a);fixture_init(&b);for(uint32_t i=0u;i<3u;++i){uint64_t sequence=orders[order][i]==EVENT_DATA||orders[order][i]==EVENT_ACK?55u:0u;CHECK(inject_and_run(&a,orders[order][i],sequence)==0);CHECK(inject_and_run(&b,orders[order][i],sequence)==0);}CHECK(memcmp(&a.domain,&b.domain,sizeof(a.domain))==0);CHECK(a.outputs.count==b.outputs.count&&memcmp(a.outputs.items,b.outputs.items,(size_t)a.outputs.count*sizeof(a.outputs.items[0]))==0);counts[order]=a.outputs.count;}CHECK(counts[0]!=counts[1]&&counts[2]!=counts[3]);return 0;}

int main(void){CHECK(retry_limit()==0);CHECK(ack_before_timeout()==0);CHECK(time_before_ack()==0);CHECK(permutations()==0);for(uint32_t i=0u;i<100u;++i)CHECK(replay()==0);return 0;}
