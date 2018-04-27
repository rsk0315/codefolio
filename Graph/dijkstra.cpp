template <class Weight>
std::vector<Weight> dijkstra(const Graph<Weight> &g, size_t s) {
  std::vector<Weight> dist(g.size(), inf<Weight>);
  greater_queue<std::pair<Weight, size_t>> q;
  dist[s] = 0;
  q.emplace(0, s);
  while (!q.empty()) {
    auto p=q.top();
    q.pop();
    Weight w=p.first;
    size_t v=p.second;
    if (w > dist[v])
      continue;

    for (const Edge<Weight> &e: g[v]) {
      if (dist[e.dst] > w + e.cost) {
        dist[e.dst] = w + e.cost;
        q.emplace(dist[e.dst], e.dst);
      }
    }
  }
  return dist;
}
