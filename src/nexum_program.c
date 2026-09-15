#include <string.h>
#include "nexum_program.h"

static int find_cell(const nexum_program_t *p, uint32_t id, uint32_t *position) {
    uint32_t i;
    for (i = 0u; i < p->cell_count; ++i) if (p->cells[i].id == id) {
        if (position != NULL) *position = i;
        return 1;
    }
    return 0;
}

static int find_domain(const nexum_program_t *p, uint32_t id, uint32_t *position) {
    uint32_t i;
    for (i = 0u; i < p->state_domain_count; ++i) if (p->state_domains[i].id == id) {
        if (position != NULL) *position = i;
        return 1;
    }
    return 0;
}

static uint32_t runtime_cell_index(const nexum_program_t *p, uint32_t id) {
    uint32_t i, result = 0u;
    for (i = 0u; i < p->cell_count; ++i) if (p->cells[i].id < id) ++result;
    return result;
}

static uint32_t runtime_domain_index(const nexum_program_t *p, uint32_t id) {
    uint32_t i, result = 0u;
    for (i = 0u; i < p->state_domain_count; ++i) if (p->state_domains[i].id < id) ++result;
    return result;
}

pnp_result_t nexum_program_validate(const nexum_program_t *p) {
    uint32_t i, j;
    if (p == NULL) return PNP_ERR_INVALID_ARGUMENT;
    if (p->cell_count > NEXUM_PROGRAM_MAX_CELLS ||
        p->state_domain_count > NEXUM_PROGRAM_MAX_STATE_DOMAINS ||
        p->edge_count > NEXUM_PROGRAM_MAX_EDGES || p->input_count > NEXUM_PROGRAM_MAX_INPUTS ||
        p->output_count > NEXUM_PROGRAM_MAX_OUTPUTS) return PNP_ERR_INVALID_GRAPH;
    if ((p->reserved[0] != 0u && p->reserved[0] != NEXUM_PROGRAM_FINALIZED) ||
        p->reserved[1] != 0u || p->reserved[2] != 0u) return PNP_ERR_INVALID_GRAPH;
    for (i = 0u; i < p->state_domain_count; ++i) {
        if (p->state_domains[i].id == UINT32_MAX || p->state_domains[i].reserved != 0u)
            return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->state_domains[j].id == p->state_domains[i].id)
            return PNP_ERR_INVALID_GRAPH;
    }
    for (i = 0u; i < p->cell_count; ++i) {
        if (p->cells[i].id == PNP_EXTERNAL_NODE ||
            !find_domain(p, p->cells[i].state_domain_id, NULL) ||
            pnp_validate_config(&p->cells[i].config) != PNP_OK) return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->cells[j].id == p->cells[i].id)
            return PNP_ERR_INVALID_GRAPH;
    }
    for (i = 0u; i < p->edge_count; ++i) {
        const nexum_edge_t *e = &p->edges[i];
        if (!find_cell(p, e->source_cell_id, NULL) || e->source_port >= PNP_MAX_OUTPUTS ||
            !find_cell(p, e->destination_cell_id, NULL) ||
            e->destination_port >= PNP_GRAPH_INPUT_PORT_COUNT || e->reserved != 0u)
            return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->edges[j].source_cell_id == e->source_cell_id &&
            p->edges[j].source_port == e->source_port && p->edges[j].route_order == e->route_order)
            return PNP_ERR_INVALID_GRAPH;
    }
    for (i = 0u; i < p->input_count; ++i) {
        const nexum_input_t *in = &p->inputs[i];
        if (in->id == UINT32_MAX || !find_cell(p, in->destination_cell_id, NULL) ||
            in->destination_port >= PNP_GRAPH_INPUT_PORT_COUNT || in->reserved != 0u)
            return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->inputs[j].id == in->id) return PNP_ERR_INVALID_GRAPH;
    }
    for (i = 0u; i < p->output_count; ++i) {
        const nexum_output_t *out = &p->outputs[i];
        if (out->id >= PNP_GRAPH_INPUT_PORT_COUNT || !find_cell(p, out->source_cell_id, NULL) ||
            out->source_port >= PNP_MAX_OUTPUTS) return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->outputs[j].id == out->id) return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < p->edge_count; ++j) if (p->edges[j].source_cell_id == out->source_cell_id &&
            p->edges[j].source_port == out->source_port && p->edges[j].route_order == out->route_order)
            return PNP_ERR_INVALID_GRAPH;
        for (j = 0u; j < i; ++j) if (p->outputs[j].source_cell_id == out->source_cell_id &&
            p->outputs[j].source_port == out->source_port && p->outputs[j].route_order == out->route_order)
            return PNP_ERR_INVALID_GRAPH;
    }
    return PNP_OK;
}

pnp_result_t nexum_program_finalize(nexum_program_t *p) {
    uint32_t i,j;pnp_result_t r;
    if(p==NULL)return PNP_ERR_INVALID_ARGUMENT;
    if(p->reserved[0]==NEXUM_PROGRAM_FINALIZED)p->reserved[0]=0u;
    r=nexum_program_validate(p);if(r!=PNP_OK)return r;
    for(i=1u;i<p->input_count;++i){nexum_input_t item=p->inputs[i];j=i;while(j>0u&&p->inputs[j-1u].id>item.id){p->inputs[j]=p->inputs[j-1u];--j;}p->inputs[j]=item;}
    p->reserved[0]=NEXUM_PROGRAM_FINALIZED;return PNP_OK;
}

pnp_result_t nexum_program_requirements(const nexum_program_t *p,
                                        nexum_program_requirements_t *r) {
    pnp_result_t result;
    if (r == NULL) return PNP_ERR_INVALID_ARGUMENT;
    result = nexum_program_validate(p);
    if (result != PNP_OK) return result;
    memset(r, 0, sizeof(*r));
    r->node_count = p->cell_count;
    r->edge_count = p->edge_count + p->output_count;
    r->state_domain_count = p->state_domain_count;
    r->input_count = p->input_count;
    r->node_storage_bytes = (size_t)r->node_count * sizeof(pnp_node_t);
    r->edge_storage_bytes = (size_t)r->edge_count * sizeof(pnp_edge_t);
    r->state_domain_storage_bytes = (size_t)r->state_domain_count * sizeof(pnp_state_domain_t);
    r->runtime_storage_bytes = r->node_storage_bytes + r->edge_storage_bytes + r->state_domain_storage_bytes;
    return PNP_OK;
}

pnp_result_t nexum_program_plan(const nexum_program_t *p,nexum_resource_plan_t *plan){
    uint32_t i,j,k,max=0u;int cyclic=0;uint8_t reach[PNP_GRAPH_MAX_NODES][PNP_GRAPH_MAX_NODES];pnp_result_t r;nexum_program_requirements_t req;
    if(plan==NULL)return PNP_ERR_INVALID_ARGUMENT;r=nexum_program_requirements(p,&req);if(r!=PNP_OK)return r;memset(plan,0,sizeof(*plan));
    plan->runtime_nodes=req.node_count;plan->state_domains=req.state_domain_count;plan->routes=req.edge_count;
    plan->routing_index_entries=req.edge_count+PNP_GRAPH_ROUTE_SLOT_COUNT;plan->input_mapping_entries=p->input_count;plan->output_mapping_entries=p->output_count;plan->scratch_words=PNP_EVENT_SCRATCH_WORD_COUNT;
    plan->graph_storage_bytes=req.runtime_storage_bytes+sizeof(pnp_graph_t);
    for(i=0u;i<p->cell_count;++i)for(j=0u;j<PNP_MAX_OUTPUTS;++j){uint32_t n=0u;for(k=0u;k<p->edge_count;++k)if(p->edges[k].source_cell_id==p->cells[i].id&&p->edges[k].source_port==j)++n;for(k=0u;k<p->output_count;++k)if(p->outputs[k].source_cell_id==p->cells[i].id&&p->outputs[k].source_port==j)++n;if(n>max)max=n;}
    plan->maximum_fanout=max;
    /* Any directed cycle makes a complete queue/emission bound unproven. */
    memset(reach,0,sizeof(reach));for(i=0u;i<p->edge_count;++i)reach[runtime_cell_index(p,p->edges[i].source_cell_id)][runtime_cell_index(p,p->edges[i].destination_cell_id)]=1u;
    for(k=0u;k<p->cell_count;++k)for(i=0u;i<p->cell_count;++i)for(j=0u;j<p->cell_count;++j)if(reach[i][k]&&reach[k][j])reach[i][j]=1u;
    for(i=0u;i<p->cell_count;++i)if(reach[i][i])cyclic=1;
    if(!cyclic){plan->queue_bound=req.edge_count+1u;plan->emission_bound=req.edge_count;plan->queue_status=(uint8_t)NEXUM_BOUND_CONSERVATIVE;plan->emission_status=(uint8_t)NEXUM_BOUND_CONSERVATIVE;}
    return PNP_OK;
}

static int route_less(const nexum_program_t *p, uint32_t a, uint32_t b) {
    uint32_t as, ap, ao, ad, ai, bs, bp, bo, bd, bi;
    if (a < p->edge_count) {
        as = p->edges[a].source_cell_id; ap = p->edges[a].source_port; ao = p->edges[a].route_order;
        ad = p->edges[a].destination_cell_id; ai = p->edges[a].destination_port;
    } else {
        const nexum_output_t *out = &p->outputs[a - p->edge_count];
        as = out->source_cell_id; ap = out->source_port; ao = out->route_order; ad = UINT32_MAX; ai = out->id;
    }
    if (b < p->edge_count) {
        bs = p->edges[b].source_cell_id; bp = p->edges[b].source_port; bo = p->edges[b].route_order;
        bd = p->edges[b].destination_cell_id; bi = p->edges[b].destination_port;
    } else {
        const nexum_output_t *out = &p->outputs[b - p->edge_count];
        bs = out->source_cell_id; bp = out->source_port; bo = out->route_order; bd = UINT32_MAX; bi = out->id;
    }
    if (as != bs) return as < bs;
    if (ap != bp) return ap < bp;
    if (ao != bo) return ao < bo;
    if (ad != bd) return ad < bd;
    return ai < bi;
}

pnp_result_t nexum_program_lower(const nexum_program_t *p,
                                 const nexum_runtime_storage_t *s, pnp_graph_t *g) {
    nexum_program_requirements_t r;
    uint32_t i, j, route_count, previous_route = UINT32_MAX;
    pnp_result_t result;
    if (s == NULL || g == NULL) return PNP_ERR_INVALID_ARGUMENT;
    result = nexum_program_requirements(p, &r);
    if (result != PNP_OK) return result;
    if (s->node_capacity < r.node_count || s->edge_capacity < r.edge_count ||
        s->state_domain_capacity < r.state_domain_count ||
        (r.node_count != 0u && s->nodes == NULL) || (r.edge_count != 0u && s->edges == NULL) ||
        (r.state_domain_count != 0u && s->state_domains == NULL)) return PNP_ERR_BUFFER_FULL;
    memset(g, 0, sizeof(*g));
    for (i = 0u; i < p->cell_count; ++i) {
        uint32_t selected = UINT32_MAX, selected_id = UINT32_MAX;
        for (j = 0u; j < p->cell_count; ++j) if (p->cells[j].id < selected_id) {
            uint32_t rank = runtime_cell_index(p, p->cells[j].id);
            if (rank == i) { selected = j; selected_id = p->cells[j].id; }
        }
        s->nodes[i].config = p->cells[selected].config;
        s->nodes[i].state_domain = runtime_domain_index(p, p->cells[selected].state_domain_id);
        s->nodes[i].reserved = 0u;
    }
    for (i = 0u; i < p->state_domain_count; ++i) for (j = 0u; j < p->state_domain_count; ++j)
        if (runtime_domain_index(p, p->state_domains[j].id) == i)
            s->state_domains[i].state = p->state_domains[j].initial_state;
    route_count = r.edge_count;
    for (i = 0u; i < route_count; ++i) {
        uint32_t candidate = UINT32_MAX;
        for (j = 0u; j < route_count; ++j)
            if ((previous_route == UINT32_MAX || route_less(p, previous_route, j)) &&
                (candidate == UINT32_MAX || route_less(p, j, candidate))) candidate = j;
        if (candidate < p->edge_count) {
            const nexum_edge_t *e = &p->edges[candidate];
            s->edges[i] = (pnp_edge_t){runtime_cell_index(p, e->source_cell_id), e->source_port,
                runtime_cell_index(p, e->destination_cell_id), e->destination_port};
        } else {
            const nexum_output_t *out = &p->outputs[candidate - p->edge_count];
            s->edges[i] = (pnp_edge_t){runtime_cell_index(p, out->source_cell_id), out->source_port,
                PNP_EXTERNAL_NODE, out->id};
        }
        previous_route = candidate;
    }
    g->nodes = s->nodes; g->node_count = r.node_count; g->node_capacity = s->node_capacity;
    g->edges = s->edges; g->edge_count = r.edge_count; g->edge_capacity = s->edge_capacity;
    g->state_domains = s->state_domains; g->state_domain_count = r.state_domain_count;
    g->state_domain_capacity = s->state_domain_capacity;
    return PNP_OK;
}

pnp_result_t nexum_program_inject(const nexum_program_t *p, pnp_queue_t *q,
                                  uint32_t input_id, const pnp_event_t *event) {
    uint32_t lo=0u,hi,mid;
    if(p==NULL)return PNP_ERR_INVALID_ARGUMENT;
    if(p->reserved[0]!=NEXUM_PROGRAM_FINALIZED){pnp_result_t result=nexum_program_validate(p);if(result!=PNP_OK)return result;
        for(mid=0u;mid<p->input_count;++mid)if(p->inputs[mid].id==input_id)return pnp_graph_inject(q,runtime_cell_index(p,p->inputs[mid].destination_cell_id),p->inputs[mid].destination_port,event);return PNP_ERR_INVALID_ARGUMENT;}
    hi=p->input_count;while(lo<hi){mid=lo+(hi-lo)/2u;if(p->inputs[mid].id<input_id)lo=mid+1u;else hi=mid;}
    if(lo<p->input_count&&p->inputs[lo].id==input_id)return pnp_graph_inject(q,runtime_cell_index(p,p->inputs[lo].destination_cell_id),p->inputs[lo].destination_port,event);
    return PNP_ERR_INVALID_ARGUMENT;
}
