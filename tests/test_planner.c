#include "test_support.h"
#include "nexum_program.h"

typedef struct runtime_fixture {
    pnp_node_t nodes[PNP_GRAPH_MAX_NODES];
    pnp_edge_t edges[PNP_GRAPH_MAX_EDGES];
    pnp_state_domain_t domains[1];
    pnp_graph_event_t queue_items[512];
    pnp_graph_output_t output_items[512];
} runtime_fixture_t;

static runtime_fixture_t runtime_fixture;

static void init_program(nexum_program_t *p,uint32_t nodes){
    uint32_t i;memset(p,0,sizeof(*p));p->cell_count=nodes;p->state_domain_count=1u;p->state_domains[0].id=0u;
    for(i=0u;i<nodes;++i){p->cells[i].id=i;p->cells[i].state_domain_id=0u;p->cells[i].config=pass_config(1u);}
}

static void add_edge(nexum_program_t *p,uint32_t source,uint32_t port,uint32_t destination){
    uint32_t i,order=0u;for(i=0u;i<p->edge_count;++i)if(p->edges[i].source_cell_id==source&&p->edges[i].source_port==port)++order;
    for(i=0u;i<p->output_count;++i)if(p->outputs[i].source_cell_id==source&&p->outputs[i].source_port==port)++order;
    p->edges[p->edge_count++]=(nexum_edge_t){source,port,destination,0u,order,0u};
}

static void add_output(nexum_program_t *p,uint32_t source,uint32_t port){
    uint32_t i,order=0u;for(i=0u;i<p->edge_count;++i)if(p->edges[i].source_cell_id==source&&p->edges[i].source_port==port)++order;
    for(i=0u;i<p->output_count;++i)if(p->outputs[i].source_cell_id==source&&p->outputs[i].source_port==port)++order;
    p->outputs[p->output_count]=(nexum_output_t){p->output_count,source,port,order};++p->output_count;
}

static void add_input(nexum_program_t *p,uint32_t destination){p->inputs[p->input_count]=(nexum_input_t){p->input_count,destination,0u,0u};++p->input_count;}

static int actual_input(nexum_program_t *p,uint32_t input_id,const pnp_event_t *event,uint64_t *emissions,uint64_t *steps,uint32_t *high_water){
    nexum_runtime_storage_t storage={runtime_fixture.nodes,PNP_GRAPH_MAX_NODES,runtime_fixture.edges,PNP_GRAPH_MAX_EDGES,runtime_fixture.domains,1u};
    pnp_graph_t graph;pnp_queue_t queue;pnp_graph_output_buffer_t outputs={runtime_fixture.output_items,512u,0u};pnp_run_stats_t stats;
    memset(&runtime_fixture,0,sizeof(runtime_fixture));storage.nodes=runtime_fixture.nodes;storage.edges=runtime_fixture.edges;storage.state_domains=runtime_fixture.domains;
    CHECK(nexum_program_lower(p,&storage,&graph)==PNP_OK);pnp_queue_init(&queue,runtime_fixture.queue_items,512u);
    CHECK(nexum_program_inject(p,&queue,input_id,event)==PNP_OK);
    CHECK(pnp_graph_run(&graph,&queue,&outputs,(pnp_run_limits_t){UINT64_MAX,UINT64_MAX},&stats)==PNP_OK);
    *emissions=stats.emitted_events;*steps=stats.steps;*high_water=stats.queue_high_water;return 0;
}

static int verify_runtime_bound(nexum_program_t *p,const pnp_event_t *event,uint64_t expected){
    nexum_resource_plan_t plan;uint64_t emissions,steps;uint32_t high_water;
    CHECK(nexum_program_plan(p,&plan)==PNP_OK);CHECK(plan.emission_status!=NEXUM_BOUND_NOT_PROVEN);
    CHECK(actual_input(p,0u,event,&emissions,&steps,&high_water)==0);CHECK(emissions<=plan.emission_bound);CHECK(steps<=plan.step_bound);CHECK(high_water<=plan.queue_bound);
    if(plan.emission_status==NEXUM_BOUND_PROVEN)CHECK(emissions==plan.emission_bound);
    if(expected!=UINT64_MAX)CHECK(plan.emission_bound==expected);
    return 0;
}

static int shapes(void){
    nexum_program_t p;pnp_event_t event={0};nexum_resource_plan_t plan;nexum_program_requirements_t requirements;
    init_program(&p,4u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,0u,0u,2u);add_edge(&p,1u,0u,3u);add_edge(&p,2u,0u,3u);add_output(&p,3u,0u);CHECK(verify_runtime_bound(&p,&event,6u)==0);
    CHECK(nexum_program_plan(&p,&plan)==PNP_OK&&nexum_program_requirements(&p,&requirements)==PNP_OK);
    CHECK(plan.runtime_nodes==4u&&plan.state_domains==1u&&plan.routes==5u&&plan.input_mapping_entries==1u&&plan.output_mapping_entries==1u);
    CHECK(plan.routing_index_entries==PNP_GRAPH_ROUTE_SLOT_COUNT+5u&&plan.routing_work_entries==PNP_GRAPH_ROUTE_SLOT_COUNT&&plan.maximum_fanout==2u&&plan.scratch_words==PNP_EVENT_SCRATCH_WORD_COUNT);
    CHECK(plan.emission_bound==6u&&plan.step_bound==5u&&plan.queue_bound==5u&&plan.emission_status==NEXUM_BOUND_PROVEN&&plan.step_status==NEXUM_BOUND_PROVEN&&plan.queue_status==NEXUM_BOUND_CONSERVATIVE);
    CHECK(plan.graph_storage_bytes==requirements.runtime_storage_bytes+sizeof(pnp_graph_t)&&plan.routing_index_storage_bytes==(size_t)plan.routing_index_entries*sizeof(uint32_t)&&plan.routing_work_storage_bytes==(size_t)plan.routing_work_entries*sizeof(uint32_t));
    init_program(&p,7u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,0u,0u,2u);add_edge(&p,1u,0u,3u);add_edge(&p,2u,0u,3u);add_edge(&p,3u,0u,4u);add_edge(&p,3u,0u,5u);add_edge(&p,4u,0u,6u);add_edge(&p,5u,0u,6u);add_output(&p,6u,0u);CHECK(verify_runtime_bound(&p,&event,16u)==0);
    init_program(&p,3u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,0u,0u,2u);add_output(&p,1u,0u);add_output(&p,2u,0u);CHECK(verify_runtime_bound(&p,&event,4u)==0);
    init_program(&p,4u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,1u,0u,2u);add_edge(&p,2u,0u,3u);add_output(&p,3u,0u);CHECK(verify_runtime_bound(&p,&event,4u)==0);
    return 0;
}

static int predicates_and_inputs(void){
    nexum_program_t p;pnp_event_t event={0};nexum_resource_plan_t plan;uint64_t emissions,steps;uint32_t high_water;
    init_program(&p,4u);add_input(&p,0u);p.cells[0].config.source_a.kind=(uint8_t)PNP_SRC_EVENT_FIELD;p.cells[0].config.predicate=(uint8_t)PNP_PRED_EQ;p.cells[0].config.compare_source.kind=(uint8_t)PNP_SRC_ONE;p.cells[0].config.emit_false=2u;
    add_output(&p,0u,0u);add_output(&p,0u,1u);add_output(&p,0u,1u);add_output(&p,0u,1u);
    CHECK(nexum_program_plan(&p,&plan)==PNP_OK&&plan.emission_bound==3u&&plan.emission_status==NEXUM_BOUND_CONSERVATIVE&&plan.maximum_fanout==3u);
    event.fields[0]=1u;CHECK(verify_runtime_bound(&p,&event,3u)==0);event.fields[0]=0u;CHECK(verify_runtime_bound(&p,&event,3u)==0);
    init_program(&p,4u);add_input(&p,0u);add_input(&p,1u);add_output(&p,0u,0u);add_edge(&p,1u,0u,2u);add_edge(&p,1u,0u,3u);add_output(&p,2u,0u);add_output(&p,3u,0u);
    CHECK(nexum_program_plan(&p,&plan)==PNP_OK&&plan.emission_bound==4u);
    CHECK(actual_input(&p,1u,&event,&emissions,&steps,&high_water)==0&&emissions==4u&&emissions<=plan.emission_bound);return 0;
}

static int layered_and_generated(void){
    nexum_program_t p;pnp_event_t event={0};uint32_t seed;
    init_program(&p,9u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,0u,0u,2u);
    for(uint32_t layer=1u;layer<4u;++layer){uint32_t a=1u+(layer-1u)*2u,b=a+1u,c=a+2u,d=a+3u;add_edge(&p,a,0u,c);add_edge(&p,a,0u,d);add_edge(&p,b,0u,c);add_edge(&p,b,0u,d);}add_output(&p,7u,0u);add_output(&p,8u,0u);CHECK(verify_runtime_bound(&p,&event,UINT64_MAX)==0);
    for(seed=1u;seed<=64u;++seed){uint32_t i,j;init_program(&p,8u);add_input(&p,0u);for(i=0u;i<7u;++i)for(j=i+1u;j<8u;++j)if(((seed+i*17u+j*13u)%(j-i+2u))==0u)add_edge(&p,i,0u,j);add_output(&p,7u,0u);CHECK(verify_runtime_bound(&p,&event,UINT64_MAX)==0);}
    return 0;
}

static int cycles_and_overflow(void){
    static nexum_program_t p;nexum_resource_plan_t plan;uint32_t i;
    init_program(&p,2u);add_input(&p,0u);add_edge(&p,0u,0u,1u);add_edge(&p,1u,0u,0u);CHECK(nexum_program_plan(&p,&plan)==PNP_OK&&plan.maximum_fanout==1u&&plan.emission_status==NEXUM_BOUND_NOT_PROVEN&&plan.step_status==NEXUM_BOUND_NOT_PROVEN&&plan.queue_status==NEXUM_BOUND_NOT_PROVEN);
    init_program(&p,256u);add_input(&p,0u);for(i=0u;i<255u;++i){add_edge(&p,i,0u,i+1u);add_edge(&p,i,0u,i+1u);}add_output(&p,255u,0u);
    CHECK(nexum_program_plan(&p,&plan)==PNP_OK&&plan.emission_status==NEXUM_BOUND_NOT_PROVEN&&plan.emission_bound==0u&&plan.step_status==NEXUM_BOUND_NOT_PROVEN&&plan.queue_status==NEXUM_BOUND_NOT_PROVEN);return 0;
}

int main(void){CHECK(shapes()==0);CHECK(predicates_and_inputs()==0);CHECK(layered_and_generated()==0);CHECK(cycles_and_overflow()==0);return 0;}
