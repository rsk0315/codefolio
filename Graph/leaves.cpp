template <class Weight>
void leaves(const graph<Weight> &g, size_t v, size_t pv,
                           std::vector<size_t>& res) {
  if (g[v].size() == 1) {
    res.push_back(v);
    return;
  }
  for (const auto& e: g[v]) {
    if (e.dst == pv) continue;
    leaves(g, e.dst, v, res);
  }
}

template <class Weight>
std::vector<size_t> leaves(const graph<Weight>& g, size_t v, size_t pv) {
  // leaves in undirected tree g
  std::vector<size_t> res;
  leaves(g, v, pv, res);
  return res;
}
