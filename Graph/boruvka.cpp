template <class Weight>
Weight min_spanning_tree(const graph<Weight>& g) {
  // based on Boruvka algorithm
  size_t v = g.size();

  intmax_t res = 0;
  union_find uf(v);
  while (uf.size(0) < v) {
    std::vector<edge<Weight>> min(v, edge<Weight>(-1, -1, inf<Weight>));
    for (size_t i = 0; i < v; ++i)
      for (const auto& e: g[i]) {
        if (uf.connected(e.src, e.dst)) continue;
        size_t s = uf.find(e.src);
        if (min[s].cost > e.cost) min[s] = e;
      }

    for (const auto& e: min) {
      if (e.src+1 == 0) continue;
      if (uf.connected(e.src, e.dst)) continue;
      uf.unite(e.src, e.dst);
      res += e.cost;
    }
  }
  return res;
}
