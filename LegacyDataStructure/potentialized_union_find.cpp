template <class Weight>
class potentialized_union_find {
  std::vector<intmax_t> tree;
  std::vector<Weight> cost;

public:
  potentialized_union_find(size_t n): tree(n, -1), cost(n) {}

  size_t find(size_t v) {
    if (tree[v] < 0) return v;

    size_t r = find(tree[v]);
    cost[v] += cost[tree[v]];
    return (tree[v] = r);
  }

  bool unite(size_t u, size_t v, Weight w) {
    size_t ru = find(u);
    size_t rv = find(v);
    if (ru == rv) {
      if (cost[u]-cost[v] != w) throw std::logic_error("inconsistent potentials");
      return false;
    }

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

  Weight potential(size_t u, size_t v) {
    if (!connected(u, v)) throw std::logic_error("an indefinite potential");
    return cost[u]-cost[v];
  }

  size_t size(size_t v) {
    return -tree[find(v)];
  }
};
