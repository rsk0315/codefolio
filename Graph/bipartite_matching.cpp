template <class Weight>
size_t bipartite_match(const Graph<Weight> &g, size_t mid) { 
  std::vector<size_t> pair(g.size(), -1);
  std::function<bool (size_t, std::vector<bool> &)> augment;
  augment = [&](size_t u, std::vector<bool> &visited) {
    if (u+1 == 0) return true;

    for (const Edge<Weight> &e: g[u]) {
      if (visited[e.dst]) continue;
      visited[e.dst] = true;
      if (augment(pair[e.dst], visited)) {
        pair[e.src] = e.dst;
        pair[e.dst] = e.src;
        return true;
      }
    }
    return false;
  };

  size_t match=0;
  for (size_t i=0; i<mid; ++i) {
    std::vector<bool> visited(g.size());
    if (augment(i, visited))
      ++match;
  }
  return match;
}

