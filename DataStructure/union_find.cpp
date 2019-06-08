class union_find {
  mutable std::vector<intmax_t> c;

public:
  union_find(size_t n): c(n, -1) {}

  size_t find(size_t v) const {
    if (c[v] < 0) return v;
    return (c[v] = find(c[v]));
  }

  bool unite(size_t u, size_t v) {
    u = find(u);
    v = find(v);
    if (u == v) return false;
    if (-c[u] > -c[v]) std::swap(u, v);
    c[v] += c[u];
    c[u] = v;
    return true;
  }

  bool connected(size_t u, size_t v) const { return (find(u) == find(v)); }

  size_t size() const { return c.size(); }
  size_t size(size_t v) const {
    return -c[find(v)];
  }
};
