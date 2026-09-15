#include <string.h>
#include "test_support.h"
#include "nexum_program.h"

static void make_program(nexum_program_t *p) {
    memset(p,0,sizeof(*p)); p->cell_count=2u;p->state_domain_count=2u;p->edge_count=2u;p->input_count=1u;p->output_count=2u;
    p->state_domains[0].id=20u;p->state_domains[1].id=10u;
    p->cells[0].id=20u;p->cells[0].state_domain_id=20u;p->cells[0].config=pass_config(3u);
    p->cells[1].id=10u;p->cells[1].state_domain_id=10u;p->cells[1].config=pass_config(1u);
    p->edges[0]=(nexum_edge_t){20u,0u,10u,7u,3u,0u};p->edges[1]=(nexum_edge_t){20u,0u,10u,8u,1u,0u};
    p->inputs[0]=(nexum_input_t){99u,20u,4u,0u};
    p->outputs[0]=(nexum_output_t){255u,20u,1u,0u};p->outputs[1]=(nexum_output_t){7u,10u,0u,0u};
}
static int lowering(void){nexum_program_t p;pnp_node_t na[2],nb[2];pnp_edge_t ea[4],eb[4];pnp_state_domain_t da[2],db[2];pnp_graph_t ga,gb;nexum_runtime_storage_t a={na,2u,ea,4u,da,2u},b={nb,2u,eb,4u,db,2u};make_program(&p);CHECK(nexum_program_lower(&p,&a,&ga)==PNP_OK);CHECK(nexum_program_lower(&p,&b,&gb)==PNP_OK);CHECK(memcmp(na,nb,sizeof(na))==0&&memcmp(ea,eb,sizeof(ea))==0&&memcmp(da,db,sizeof(da))==0);CHECK(ea[0].destination_node==PNP_EXTERNAL_NODE&&ea[0].destination_port==7u);CHECK(ea[1].destination_port==8u&&ea[2].destination_port==7u);CHECK(ea[3].destination_node==PNP_EXTERNAL_NODE&&ea[3].destination_port==255u);p.outputs[0].id=256u;CHECK(nexum_program_validate(&p)==PNP_ERR_INVALID_GRAPH);return 0;}
static int injection_validates(void){nexum_program_t p;pnp_graph_event_t item;pnp_queue_t q;pnp_event_t e={0};make_program(&p);pnp_queue_init(&q,&item,1u);CHECK(nexum_program_inject(&p,&q,99u,&e)==PNP_OK);p.reserved[0]=1u;pnp_queue_init(&q,&item,1u);CHECK(nexum_program_inject(&p,&q,99u,&e)==PNP_ERR_INVALID_GRAPH&&q.count==0u);return 0;}
static int partial_commit(void){pnp_node_t n[2];pnp_state_domain_t d[2];pnp_edge_t edges[2];pnp_graph_t g={n,2u,2u,edges,2u,2u,d,2u,2u};pnp_graph_event_t store[1];pnp_queue_t q;pnp_event_t e={0};pnp_run_stats_t stats;pnp_graph_output_buffer_t out={0};memset(n,0,sizeof(n));memset(d,0,sizeof(d));n[0].config=pass_config(1u);n[0].config.update_mode=(uint8_t)PNP_UPDATE_CONSTANT;n[0].config.update_constant=9u;n[1].config=pass_config(0u);n[1].state_domain=1u;edges[0]=(pnp_edge_t){0u,0u,1u,0u};edges[1]=(pnp_edge_t){0u,0u,1u,1u};pnp_queue_init(&q,store,1u);CHECK(pnp_graph_inject(&q,0u,0u,&e)==PNP_OK);CHECK(pnp_graph_run(&g,&q,&out,(pnp_run_limits_t){3u,3u},&stats)==PNP_ERR_QUEUE_FULL);CHECK(d[0].state.words[0]==9u&&q.count==1u&&stats.steps==1u&&stats.emitted_events==1u);return 0;}
int main(void){CHECK(lowering()==0);CHECK(injection_validates()==0);CHECK(partial_commit()==0);return 0;}
