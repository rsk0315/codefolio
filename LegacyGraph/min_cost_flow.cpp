template <class Tp>
struct edge {
  size_t dst;
  Tp cap, cost;
  size_t rev;
  edge(size_t dst, Tp cap, Tp cost, size_t rev):
    dst(dst), cap(cap), cost(cost), rev(rev)
  {}
};

template <class Tp>
struct graph: public std::vector<std::vector<edge<Tp>>> {
  graph(size_t n): std::vector<std::vector<edge<Tp>>>(n) {}

  void connect_to(size_t src, size_t dst, Tp cost) {
    (*this)[src].emplace_back(dst, 1, cost, (*this)[dst].size());
    (*this)[dst].emplace_back(src, 0, -cost, (*this)[src].size()-1);
  }
};

template <class Tp>
Tp inf() { return 1 << 29; }

template <class Tp>
Tp mincost_flow(graph<Tp>& g, size_t s, size_t t, Tp f) {
  Tp res = 0;
  Tp max = inf<Tp>();
  while (f > 0) {
    std::vector<Tp> dist(g.size(), inf<Tp>());
    std::vector<size_t> pv(g.size()), pe(g.size());
    dist[s] = 0;
    bool updated = true;
    while (updated) {
      updated = false;
      for (size_t v = 0; v < g.size(); ++v) {
        if (dist[v] == max) continue;

        for (size_t i = 0; i < g[v].size(); ++i) {
          edge<Tp>& e = g[v][i];
          if (e.cap > 0 && dist[e.dst] > dist[v] + e.cost) {
            dist[e.dst] = dist[v] + e.cost;
            pv[e.dst] = v;
            pe[e.dst] = i;
            updated = true;
          }
        }
      }
    }

    if (dist[t] == max) return -1;
    Tp d = f;
    for (size_t v = t; v != s; v = pv[v])
      d = std::min(d, g[pv[v]][pe[v]].cap);

    f -= d;
    res += d * dist[t];
    for (size_t v = t; v != s; v = pv[v]) {
      edge<Tp>& e = g[pv[v]][pe[v]];
      e.cap -= d;
      g[v][e.rev].cap += d;
    }
  }
  return res;
}
