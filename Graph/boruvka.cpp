template <class Weight>
Weight min_spanning_tree(const graph<Weight>& g) {
  // based on Boruvka algorithm
  size_t v = g.size();

  intmax_t res = 0;
  union_find uf(v);
  while (uf.size(0) < v) {
    std::vector<std::pair<size_t, size_t>> min(v, {-1, -1});
    for (size_t i = 0; i < v; ++i)
      for (size_t j = 0; j < g[i].size(); ++j) {
        const auto& e = g[i][j];
        if (uf.connected(e.src, e.dst)) continue;
        size_t s = uf.find(e.src);
        size_t u0, u1;
        std::tie(u0, u1) = min[s];
        if (u0+1 == 0 || g[u0][u1].cost > e.cost) min[s] = {i, j};
      }

    for (size_t i = 0; i < v; ++i) {
      size_t u0, u1;
      std::tie(u0, u1) = min[i];
      if (u0+1 == 0) continue;
      const auto& e = g[u0][u1];
      if (uf.connected(e.src, e.dst)) continue;
      uf.unite(e.src, e.dst);
      res += e.cost;
    }
  }
  return res;
}
