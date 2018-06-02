template <class Weight>
class WeightedUnionFind {
  std::vector<intmax_t> tree;
  std::vector<Weight> cost;

public:
  WeightedUnionFind(size_t n): tree(n, -1), cost(n) {}

  size_t find(size_t v) {
    if (tree[v] < 0)
      return v;

    size_t r=find(tree[v]);
    cost[v] += cost[tree[v]];
    return (tree[v] = r);
  }

  bool unite(size_t u, size_t v, Weight w) {
    size_t ru=find(u), rv=find(v);
    if (ru == rv)
      return false;  // FIXME assuming no multi-edges

    w -= cost[u]-cost[v];
    if (-tree[ru] >= -tree[rv]) {
      std::swap(ru, rv);
      std::swap(u, v);
      w = -w;
    }
    tree[rv] += tree[ru];
    tree[ru] = rv;
    cost[ru] = w;
    return true;
  }

  bool connected(size_t u, size_t v) {
    return (find(u) == find(v));
  }

  Weight diff(size_t u, size_t v) {
    return connected(u, v)? (cost[u]-cost[v]):inf<Weight>;
  }

  size_t size(size_t v) {
    return -tree[find(v)];
  }
};
