template <class Weight>
struct edge {
  size_t src, dst;
  Weight cost;

  edge(size_t src, size_t dst, Weight cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const edge<Weight> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
struct graph: public std::vector<std::vector<edge<Weight>>> {
  graph(size_t n): std::vector<std::vector<edge<Weight>>>(n) {}

  void connect_with(size_t src, size_t dst, Weight cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
    (*this)[dst].emplace_back(dst, src, cost);
  }

  void connect_to(size_t src, size_t dst, Weight cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
  }

  std::vector<size_t> sccomp(size_t &num) const {
    const size_t V=this->size();
    graph rev(V);
    for (const auto &v: *this)
      for (const auto &e: v)
        rev.connect_to(e.dst, e.src, e.cost);

    std::vector<bool> used(V);
    std::vector<size_t> vs;
    std::function<void (size_t)> dfs=[&](size_t v) {
      used[v] = true;
      for (size_t i=0; i<(*this)[v].size(); ++i)
        if (!used[(*this)[v][i].dst]) dfs((*this)[v][i].dst);

      vs.push_back(v);
    };

    for (size_t v=0; v<V; ++v)
      if (!used[v]) dfs(v);

    used.assign(V, false);
    std::vector<size_t> cmp(V);
    num = 0;
    std::function<void (size_t)> rdfs=[&](size_t v) {
      used[v] = true;
      cmp[v] = num;
      for (size_t i=0; i<rev[v].size(); ++i)
        if (!used[rev[v][i].dst]) rdfs(rev[v][i].dst);
    };

    for (size_t i=vs.size(); i--;)
      if (!used[vs[i]]) {
        rdfs(vs[i]);
        ++num;
      }

    return cmp;
  }

  std::vector<size_t> sccomp() const {
    size_t tmp;
    return sccomp(tmp);
  }
};
