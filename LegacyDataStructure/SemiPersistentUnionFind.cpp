template <class Tp>
class semi_persistent_array {
  using Node = std::pair<size_t, Tp>;  // <timestamp, value>
  std::vector<std::vector<Node>> entity;
  size_t last = 0;

public:
  semi_persistent_array(size_t n, Tp v=Tp()):
    entity(n, std::vector<Node>(1, {0, v}))
  {}

  void update(size_t i, Tp x, size_t t) {
    assert(last <= t);
    last = t;
    if (entity[i].back().first == t) {
      entity[i].back().second = x;
    } else {
      assert(entity[i].back().first < t);
      entity[i].emplace_back(t, x);
    }
  }

  Tp get(size_t i, size_t t=-1) const {
    if (entity[i].back().first <= t) {
      // most-frequent case
      return entity[i].back().second;
    }

    size_t lb = 0;
    size_t ub = entity[i].size();
    while (ub-lb > 1) {
      size_t mid = (lb+ub)>>1;
      ((entity[i][mid].first <= t)? lb:ub) = mid;
    }
    return entity[i][lb].second;
  }
};

class semi_persistent_union_find {
  semi_persistent_array<intmax_t> tree;
  size_t last = 0;

  size_t find_root(size_t v, size_t t) const {
    intmax_t pv = tree.get(v, t);
    while (pv >= 0) {
      v = pv;
      pv = tree.get(pv, t);
    }
    return v;
  }

public:
  semi_persistent_union_find(size_t n): tree(n, -1) {}

  bool unite(size_t u, size_t v, size_t t=-1) {
    if (t+1 != 0) {
      assert(last <= t);
      last = t;
    }
    u = find_root(u, t);
    v = find_root(v, t);
    if (u == v) return false;
    size_t su = -tree.get(u, t);
    size_t sv = -tree.get(v, t);
    if (su == sv) {
      tree.update(v, -sv-1, t);
    } else if (su > sv) {
      std::swap(u, v);
    }
    tree.update(u, v, t);
    return true;
  }

  bool connected(size_t u, size_t v, size_t t=-1) const {
    return find_root(u, t) == find_root(v, t);
  }
};
