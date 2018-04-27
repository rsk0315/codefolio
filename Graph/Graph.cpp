template <class Weight>
struct Edge {
  size_t src, dst;
  Weight cost;

  Edge(size_t src, size_t dst, Weight cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const Edge<Weight> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
using Graph=std::vector<std::vector<Edge<Weight>>>;
