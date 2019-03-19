template <class Weight>
struct edge {
  using value_type = Weight;
  size_t src, dst;
  value_type cost;

  edge(size_t src, size_t dst, value_type cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const edge<value_type> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
struct graph: public std::vector<std::vector<edge<Weight>>> {
  using value_type = Weight;
  graph(size_t n): std::vector<std::vector<edge<value_type>>>(n) {}

  void connect_with(size_t src, size_t dst, value_type cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
    (*this)[dst].emplace_back(dst, src, cost);
  }
};
