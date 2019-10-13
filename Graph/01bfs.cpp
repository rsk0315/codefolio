struct bfs01_tag {} bfs01;

template <typename Edge, typename Directedness>
auto shortest(adjacency_list<Edge, Directedness> const& g, size_t s, bfs01_tag) {
  using weight_type = typename Edge::weight_type;
  size_t n = g.size();
  std::vector<weight_type> dp(n, g.inf);
  dp[s] = weight_type(0);
  std::deque<size_t> dq{s};
  while (!dq.empty()) {
    size_t v = dq.front();
    dq.pop_front();
    for (auto const& e: g[v]) {
      size_t nv = e.target();
      weight_type nw = dp[e.source()];
      bool zero = (e.weight() == weight_type(0));
      if (!zero) ++nw;
      if (!(nw < dp[nv])) continue;
      dp[nv] = nw;
      if (zero) {
        dq.push_front(nv);
      } else {
        dq.push_back(nv);
      }
    }
  }
  return dp;
}
