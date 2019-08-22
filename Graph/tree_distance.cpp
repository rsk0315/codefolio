template <typename Tree>
auto tree_distance(Tree const& g, size_t s) {
  using weight_type = typename Tree::weight_type;
  std::vector<weight_type> res(g.size());
  make_fix_point([&](auto dfs, size_t v, size_t p) -> void {
      for (auto const& e: g[v]) {
        size_t nv = e.target();
        if (nv == p) continue;
        res[nv] = res[v] + e.weight();
        dfs(nv, v);
      }
  })(s, -1);
  return res;
}
