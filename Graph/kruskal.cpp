#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>

template <class T>
constexpr T inf=(T(1)<<(8*sizeof(T)-3));

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

class UnionFind {
  std::vector<intmax_t> tree;

public:
  UnionFind(size_t n): tree(n, -1) {}

  size_t find(size_t v) {
    if (tree[v] < 0)
      return v;

    return (tree[v] = find(tree[v]));
  }

  bool unite(size_t u, size_t v) {
    u = find(u);
    v = find(v);
    if (u == v)
      return false;

    if (-tree[u] >= -tree[v])
      std::swap(u, v);

    tree[v] += tree[u];
    tree[u] = v;
    return true;
  }

  bool connected(size_t u, size_t v) {
    return (find(u) == find(v));
  }

  size_t size(size_t v) {
    return -tree[find(v)];
  }
};

template <class Weight>
Weight cost_mst(size_t v, const std::vector<Edge<Weight>> &es) {
  // assert(std::is_sorted(es.begin(), es.end()));

  UnionFind uf(v);
  Weight res=0;
  for (const Edge<Weight> &e: es) {
    if (uf.connected(e.src, e.dst)) continue;

    res += e.cost;
    uf.unite(e.src, e.dst);
  }

  return res;
}
