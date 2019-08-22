template <class Weight>
std::vector<Weight> shortest(const graph<Weight>& g, size_t s) {
  // based on Dijkstra algorithm
  std::vector<Weight> res(g.size(), inf<Weight>);
  priority_queue<std::pair<Weight, size_t>, std::greater<>> q;
  res[s] = 0;
  q.emplace(0, s);
  while (!q.empty()) {
    Weight w;
    size_t v;
    std::tie(w, v) = q.top();
    q.pop();
    if (w > res[v]) continue;

    for (const auto& e: g[v]) {
      if (res[e.dst] > w + e.cost) {
        res[e.dst] = w + e.cost;
        q.emplace(res[e.dst], e.dst);
      }
    }
  }
  return res;
}
