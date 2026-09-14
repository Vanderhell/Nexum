#include "test_support.h"
int main(void){
    pnp_node_t n[3];pnp_state_domain_t d[3];pnp_edge_t e[3];pnp_graph_t g={n,3u,3u,e,2u,3u,d,3u,3u};pnp_graph_analysis_t a;
    memset(n,0,sizeof(n));memset(d,0,sizeof(d));for(uint32_t i=0u;i<3u;++i){n[i].config=pass_config(1u);n[i].state_domain=i;}
    e[0]=(pnp_edge_t){0u,0u,1u,0u};e[1]=(pnp_edge_t){1u,0u,2u,0u};CHECK(pnp_graph_analyze(&g,4u,&a)==PNP_OK);CHECK(!a.has_cycles&&a.d3_status==PNP_D3_PROVEN&&a.state_bytes==sizeof(d));
    n[1].state_domain=0u;CHECK(pnp_graph_analyze(&g,4u,&a)==PNP_OK&&a.d3_status==PNP_D3_NOT_PROVEN);n[1].state_domain=1u;
    e[1].destination_node=0u;CHECK(pnp_graph_analyze(&g,4u,&a)==PNP_OK&&a.has_cycles&&a.d3_status==PNP_D3_NOT_PROVEN);
    e[1]=(pnp_edge_t){1u,0u,2u,0u};g.edge_count=3u;e[2]=(pnp_edge_t){0u,0u,2u,0u};CHECK(pnp_graph_analyze(&g,4u,&a)==PNP_OK&&a.d3_status==PNP_D3_NOT_PROVEN);return 0;
}
