#ifndef _VERTEX_COVER_H
#define _VERTEX_COVER_H

// vertex_cover() builds a list of vertices which covers every edge of the graph
// Input is a classical adjacency-list graph
// As an output, vertex_cover() modify the degrees in degs[], so that
// any vertex with a degree > 0 belongs to the vertex coverage.
// Moreover, vertex_cover() keeps links[] intact, permuting only the adjacency lists

#include "gengraph_box_list.h"

namespace gengraph { 

void vertex_cover(int n, int *links, int *deg, int **neigh = NULL) {
  int i;
  // create and initialize neigh[]
  if (neigh==NULL) {
    neigh = new int*[n];
    neigh[0] = links;
    for(i=1; i<n; i++) neigh[i]=neigh[i-1]+deg[i];
  }
  // create box_list
  box_list bl(n,deg);
  do {
    int v;
    // remove vertices adjacent to vertices of degree 1
    while((v=bl.get_one())>=0) bl.pop_vertex(v, neigh);
    // remove vertex of max degree and its highest-degree neighbour
    if(!bl.is_empty()) {
      v=bl.get_max();
      int *w = neigh[v];
      register int v2 = *(w++);
      register int dm = deg[v2];
      register int k = deg[v]-1;
      while(k--) if(deg[*(w++)]>dm) { v2 = *(w-1); dm=deg[v2]; };
      bl.pop_vertex(v, neigh);
      bl.pop_vertex(v2,neigh);
    }
  } while(!bl.is_empty());
}

} // namespace gengraph

#endif //_VERTEX_COVER_H
