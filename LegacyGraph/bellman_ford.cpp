template <class Weight>
std::vector<Weight> shortest(const graph<Weight>& g, size_t s) {
  // based-on Bellman-Ford algorithm
  size_t V = g.size();
  std::vector<Weight> res(V, inf<Weight>);
  res[s] = 0;
 
  for (size_t i = 1; i < V; ++i)
    for (size_t v = 0; v < V; ++v)
      for (const auto& e: g[v])
        if (res[e.dst] > res[e.src] + e.cost)
          res[e.dst] = res[e.src] + e.cost;
 
  for (size_t v = 0; v < V; ++v)
    for (const auto& e: g[v])
      if (res[e.dst] > res[e.src] + e.cost)
        res[e.dst] = -inf<Weight>;
  
  return res;
}
