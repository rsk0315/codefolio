template <class Potential>
class potentialized_union_find {
public:
  using value_type = Potential;

private:
  mutable std::vector<intmax_t> tree;
  mutable std::vector<value_type> cost;

public:
  potentialized_union_find(size_t n): tree(n, -1), cost(n) {}

  size_t find(size_t v) const {
    if (tree[v] < 0) return v;
    size_t r = find(tree[v]);
    cost[v] += cost[tree[v]];
    return (tree[v] = r);
  }

  bool unite(size_t u, size_t v, value_type w) {
    size_t ru = find(u), rv = find(v);
    if (ru == rv) return false;  // FIXME assuming no multi-edges

    w -= cost[u] - cost[v];
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

  bool connected(size_t u, size_t v) const {
    return (find(u) == find(v));
  }

  value_type diff(size_t u, size_t v) const {
    return connected(u, v)? (cost[u]-cost[v]): inf<value_type>;
  }

  size_t size(size_t v) const {
    return -tree[find(v)];
  }
};
