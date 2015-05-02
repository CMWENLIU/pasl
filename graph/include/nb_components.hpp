#include "edgelist.hpp"
#include "adjlist.hpp"
#include "pcontainer.hpp"

#ifndef _PASL_GRAPH_NB_COMPONENTS_H_
#define _PASL_GRAPH_NB_COMPONENTS_H_

namespace pasl {
namespace graph {

template <class Adjlist_seq>
typename adjlist<Adjlist_seq>::vtxid_type
nb_components_bfs_by_array(const adjlist<Adjlist_seq>& graph) {
  using vtxid_type = typename adjlist<Adjlist_seq>::vtxid_type;
  vtxid_type nb_vertices = graph.get_nb_vertices();
  bool* was = data::mynew_array<bool>(nb_vertices);
  fill_array_seq(was, nb_vertices, false);
  vtxid_type result = 0;
  LOG_BASIC(ALGO_PHASE);
  vtxid_type* queue = data::mynew_array<vtxid_type>(nb_vertices);
  for (vtxid_type v = 0; v < nb_vertices; ++v) {
  	if (was[v]) {
  		continue;
  	}
  	result++;
    vtxid_type head = 0;
    vtxid_type tail = 0;
  	was[v] = true;
  	queue[tail++] = v;
  	while (head < tail) {
  		vtxid_type vertex = queue[head++];
  		vtxid_type degree = graph.adjlists[vertex].get_out_degree();
  		vtxid_type* neighbors = graph.adjlists[vertex].get_out_neighbors();
  		for (vtxid_type edge = 0; edge < degree; ++edge) {
  			vtxid_type other = neighbors[edge];
  			if (was[other]) {
  				continue;
  			}
  			was[other] = true;
  			queue[tail++] = other;
  		}
  	}
  }
  free(queue);
  free(was);
  return result;
}


template <class Adjlist_seq>
void
nb_components_dfs_by_array_recursive(const adjlist<Adjlist_seq>& graph, typename adjlist<Adjlist_seq>::vtxid_type vertex, bool* was) {
  was[vertex] = true;
  using vtxid_type = typename adjlist<Adjlist_seq>::vtxid_type;
  vtxid_type degree = graph.adjlists[vertex].get_out_degree();
  vtxid_type* neighbors = graph.adjlists[vertex].get_out_neighbors();
  for (vtxid_type edge = 0; edge < degree; ++edge) {
  	vtxid_type other = neighbors[edge];
  	if (was[other]) {
  	  continue;
  	}
    nb_components_dfs_by_array_recursive(graph, other, was);
  }
  return;
}

template <class Adjlist_seq>
typename adjlist<Adjlist_seq>::vtxid_type
nb_components_dfs_by_array_recursive(const adjlist<Adjlist_seq>& graph) {
  using vtxid_type = typename adjlist<Adjlist_seq>::vtxid_type;
  vtxid_type nb_vertices = graph.get_nb_vertices();
  bool* was = data::mynew_array<bool>(nb_vertices);
  fill_array_seq(was, nb_vertices, false);
  vtxid_type result = 0;
  LOG_BASIC(ALGO_PHASE);
  for (vtxid_type v = 0; v < nb_vertices; ++v) {
  	if (was[v]) {
  		continue;
  	}
  	result ++;
  	nb_components_dfs_by_array_recursive(graph, v, was);  
  }
  free(was);
  return result;
}

template <class Adjlist_seq>
typename adjlist<Adjlist_seq>::vtxid_type
nb_components_dfs_by_array(const adjlist<Adjlist_seq>& graph) {
  using vtxid_type = typename adjlist<Adjlist_seq>::vtxid_type;
  vtxid_type nb_vertices = graph.get_nb_vertices();
  bool* was = data::mynew_array<bool>(nb_vertices);
  vtxid_type* stack = data::mynew_array<vtxid_type>(nb_vertices);
  vtxid_type* cur_edge = data::mynew_array<vtxid_type>(nb_vertices);
  fill_array_seq(was, nb_vertices, false);
  vtxid_type result = 0;
  LOG_BASIC(ALGO_PHASE);
  for (vtxid_type v = 0; v < nb_vertices; ++v) {
  	if (was[v]) {
  		continue;
  	}
  	result ++;
  	vtxid_type stack_size = 0;
  	stack[0] = v;
  	cur_edge[stack_size++] = 0;
  	was[v] = true;
  	while (stack_size != 0) {
  		vtxid_type vertex = stack[stack_size - 1];
  		vtxid_type cur_edge_id = cur_edge[stack_size - 1]++;
  		vtxid_type degree = graph.adjlists[vertex].get_out_degree();
  		if (cur_edge_id == degree) {
  			stack_size--;
  			continue;
  		}
  		vtxid_type other = graph.adjlists[vertex].get_out_neighbors()[cur_edge_id];
  		if (was[other]) {
  			continue;
  		}
  		was[other] = true;
  		stack[stack_size] = other;
  		cur_edge[stack_size++] = 0;
  	}
  }
  free(was);
  free(stack);
  free(cur_edge);
  return result;
}

template<class vtxid_type>
vtxid_type 
get_parent(vtxid_type *parent, vtxid_type vertex) {
  if (parent[vertex] != vertex) {
  	parent[vertex] = get_parent(parent, parent[vertex]);
  }
  return parent[vertex];
}

template<class vtxid_type>
bool unite(vtxid_type* parent, vtxid_type v, vtxid_type u) {
	v = get_parent(parent, v);
	u = get_parent(parent, u);
	if (v == u) {
		return false;
	}
	parent[v] = u;
	return true;
}

template <class Edge_bag>
typename edgelist<Edge_bag>::vtxid_type
nb_components_disjoint_set_union(const edgelist<Edge_bag>& graph) {
  using vtxid_type = typename edgelist<Edge_bag>::vtxid_type;
  vtxid_type nb_vertices = graph.nb_vertices;
  vtxid_type* parent = data::mynew_array<vtxid_type>(nb_vertices);
  for (vtxid_type i = 0; i != nb_vertices; ++i) {
  	parent[i] = i;
  }
  vtxid_type result = nb_vertices;
  edgeid_type nb_edges = graph.get_nb_edges();
  for (edgeid_type i = 0; i < nb_edges; ++i) {
  	if (unite(parent, graph.edges[i].src, graph.edges[i].dst)) {
  		result--;
  	}
  }
  free(parent);
  return result;
}

template <class Index, class Item>
  static bool try_to_set_dist(Index target,
                              Item unknown, Item dist,
                              std::atomic<Item>* dists) {
    if (dists[target].load(std::memory_order_relaxed) != unknown)
      return false;
    // if (idempotent)
    if (false)
      dists[target].store(dist, std::memory_order_relaxed);
    else if (! dists[target].compare_exchange_strong(unknown, dist))
      return false;
    return true;
  }

template <class Adjlist>
typename Adjlist::vtxid_type
nb_components_pbbs_pbfs(const Adjlist& graph) {
  using vtxid_type = typename Adjlist::vtxid_type;
  struct nonNegF{bool operator() (vtxid_type a) {return (a>=0);}};
  vtxid_type nb_vertices = graph.get_nb_vertices();
  edgeid_type nb_edges = graph.nb_edges;
  std::atomic<bool>* was = data::mynew_array<std::atomic<bool>>(nb_vertices);
  fill_array_par(was, nb_vertices, false);
  LOG_BASIC(ALGO_PHASE);
  vtxid_type* Frontier = data::mynew_array<vtxid_type>(nb_edges);
  vtxid_type* FrontierNext = data::mynew_array<vtxid_type>(nb_edges);
  vtxid_type* Counts = data::mynew_array<vtxid_type>(nb_vertices);
  vtxid_type result = 0;
  for (vtxid_type vertex = 0; vertex != nb_vertices; ++vertex) {
  	if (was[vertex]) {
  		continue;
  	}
  	result++;
    Frontier[0] = vertex;
    vtxid_type frontierSize = 1;
    was[vertex].store(true);
    while (frontierSize > 0) {
      sched::native::parallel_for(vtxid_type(0), frontierSize, [&] (vtxid_type i) {
        Counts[i] = graph.adjlists[Frontier[i]].get_out_degree();
      });
      vtxid_type nr = pbbs::sequence::scan(Counts,Counts,frontierSize,pbbs::utils::addF<vtxid_type>(),vtxid_type(0));
      sched::native::parallel_for(vtxid_type(0), frontierSize, [&] (vtxid_type i) {
        vtxid_type v = Frontier[i];
        vtxid_type o = Counts[i];
        vtxid_type degree = graph.adjlists[v].get_out_degree();
        vtxid_type* neighbors = graph.adjlists[v].get_out_neighbors();
        sched::native::parallel_for(vtxid_type(0), degree, [&] (vtxid_type j) {
          vtxid_type other = neighbors[j];
          if (try_to_set_dist(other, false, true, was)) {
            FrontierNext[o+j] = other;
          } else {
            FrontierNext[o+j] = vtxid_type(-1);
          }
        });
      });
      frontierSize = pbbs::sequence::filter(FrontierNext,Frontier,nr,nonNegF());
    }
  }	
  free(FrontierNext); free(Frontier); free(Counts); free(was);
  return result;
}



} // end namespace
} // end namespace

#endif /*! _PASL_GRAPH_NB_COMPONENTS_H_ */