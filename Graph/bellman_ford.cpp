template <class Weight>
std::vector<Weight> bellman_ford(const Graph<Weight> &g, size_t s, bool &nl) {
  std::vector<Weight> dist(g.size(), inf<Weight>);
  dist[s] = 0;
  nl = false;
  for (size_t k=0; k<g.size(); ++k)
    for (size_t i=0; i<g.size(); ++i)
      for (const Edge<Weight> &e: g[i]) {
        if (dist[e.dst] > dist[e.src] + e.cost) {
          dist[e.dst] = dist[e.src] + e.cost;
          if (k+1 == g.size()) {
            dist[e.dst] = -inf<Weight>;
            nl = true;
          }
        }
      }

  return dist;
}
