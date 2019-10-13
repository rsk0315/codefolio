struct dijkstra_tag {} dijkstra;

template <typename Edge, typename Directedness>
auto shortest(adjacency_list<Edge, Directedness> const& g, size_t s, dijkstra_tag) {
  using weight_type = typename Edge::weight_type;
  using pair_type = std::pair<weight_type, size_t>;
  size_t n = g.size();
  std::vector<weight_type> dp(n, std::numeric_limits<weight_type>::max());
  dp[s] = weight_type(0);
  std::priority_queue<pair_type, std::vector<pair_type>, std::greater<>> pq;
  pq.emplace(weight_type(0), s);
  while (!pq.empty()) {
    weight_type w;
    size_t v;
    std::tie(w, v) = pq.top();
    pq.pop();
    if (dp[v] < w) continue;
    for (auto const& e: g[v]) {
      weight_type nw = dp[e.source()] + e.weight();
      size_t nv = e.target();
      if (nw < dp[nv]) {
        dp[nv] = nw;
        pq.emplace(std::move(nw), nv);
      }
    }
  }
  return dp;
}
