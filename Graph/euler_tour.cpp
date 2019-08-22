template <typename Edge>
auto euler_tour(adjacency_list<Edge, undirected_tag> const& g, size_t r) {
  // .first[i]: i-th edge
  // .second[0][v]: index of tour (without r) that enters vertex v
  // .second[1][v]: index of tour that leaves vertex v
  size_t n = g.size();
  std::vector<Edge> res_first;
  std::array<std::vector<size_t>, 2> res_second;
  res_second[0] = res_second[1] = std::vector<size_t>(n);
  size_t i = 0;
  make_fix_point([&](auto dfs, size_t v, size_t p) -> void {
      Edge ep;
      for (auto const& e: g[v]) {
        if (e.target() == p) {
          ep = e;
          continue;
        }
        res_second[0][e.target()] = i++;
        res_first.push_back(e);
        dfs(e.target(), e.source());
      }
      if (v != r) {
        res_second[1][v] = i++;
        res_first.push_back(ep);
      }
  })(r, -1);
  return std::make_pair(res_first, res_second);
}
