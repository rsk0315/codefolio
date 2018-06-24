template <class T>
class SemiPersistentArray {
  using Node = std::pair<size_t, T>;  // <timestamp, value>
  std::vector<std::vector<Node>> entity;
  size_t last=0;

public:
  SemiPersistentArray(size_t n, T v=T()):
    entity(n, std::vector<Node>(1, {0, v}))
  {}

  void update(size_t i, T x, size_t t) {
    assert(last <= t);
    last = t;
    if (entity[i].back().first == t) {
      entity[i].back().second = x;
    } else {
      assert(entity[i].back().first < t);
      entity[i].emplace_back(t, x);
    }
  }

  T get(size_t i, size_t t=-1) const {
    if (entity[i].back().first <= t) {
      // most-frequent case
      return entity[i].back().second;
    }

    size_t lb=0, ub=entity[i].size();
    while (ub-lb > 1) {
      size_t mid=(lb+ub)>>1;
      ((entity[i][mid].first <= t)? lb:ub) = mid;
    }
    return entity[i][lb].second;
  }
};

class SemiPersistentUnionFind {
  SemiPersistentArray<int> tree;
  size_t last=0;

  size_t find_root(size_t v, size_t t) const {
    int pv=tree.get(v, t);
    while (pv >= 0) {
      v = pv;
      pv = tree.get(pv, t);
    }
    return v;
  }

public:
  SemiPersistentUnionFind(size_t n): tree(n, -1) {}

  bool unite(size_t u, size_t v, size_t t=-1) {
    if (t+1 != 0) {
      assert(last <= t);
      last = t;
    }
    u = find_root(u, t);
    v = find_root(v, t);
    if (u == v) return false;
    int su=-tree.get(u, t), sv=-tree.get(v, t);
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
