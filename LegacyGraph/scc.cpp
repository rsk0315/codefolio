template <class Weight>
std::vector<size_t> strongly_connected_components(const graph<Weight>& g) {
  size_t n = g.size();
  graph<int> rev(n);
  for (const auto& v: g)
    for (const auto& e: v)
      rev.connect_to(e.dst, e.src);

  std::vector<bool> used(n);
  std::vector<size_t> vs;
  auto dfs = make_fix_point([&](auto f, size_t v) -> void {
      used[v] = true;
      for (size_t i = 0; i < g[v].size(); ++i)
        if (!used[g[v][i].dst]) f(g[v][i].dst);
      vs.push_back(v);
  });
  for (size_t i = 0; i < n; ++i)
    if (!used[i]) dfs(i);

  used.assign(n, false);
  std::vector<size_t> cmp(n);
  size_t num = 0;
  auto rdfs = make_fix_point([&](auto f, size_t v) -> void {
      used[v] = true;
      cmp[v] = num;
      for (size_t i = 0; i < rev[v].size(); ++i)
        if (!used[rev[v][i].dst]) f(rev[v][i].dst);
  });

  for (size_t i = vs.size(); i--;)
    if (!used[vs[i]]) {
      rdfs(vs[i]);
      ++num;
    }

  return cmp;
}
