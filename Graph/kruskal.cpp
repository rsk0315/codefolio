template <class ForwardIt>
auto min_spanning_tree(size_t v, ForwardIt first, ForwardIt last) {
  // return type: std::pair<Weight, std::set<edge<Weight>>
  // based on Kruskal algorithm
  using Weight = typename ForwardIt::value_type::value_type;
  union_find uf(v);
  Weight cost = Weight();
  std::set<edge<Weight>> mst;
  while (first != last) {
    const auto& e = *first++;
    if (uf.connected(e.src, e.dst)) continue;
    cost += e.cost;
    mst.insert(e);
    uf.unite(e.src, e.dst);
  }
  return std::make_pair(cost, mst);
}
