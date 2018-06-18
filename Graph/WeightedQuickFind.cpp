template <class Weight>
class WeightedQuickFind {
  std::vector<std::vector<size_t>> groups;
  std::vector<size_t> ind;
  std::vector<Weight> weight;

public:
  WeightedQuickFind(size_t n): ind(n), weight(n) {
    std::iota(ind.begin(), ind.end(), 0);
    for (size_t i=0; i<n; ++i)
      groups.emplace_back(1, i);
  }

  struct NotConnected: public std::logic_error {
    NotConnected(): std::logic_error("") {}
  };

  void unite(size_t u, size_t v, Weight w) {
    // (weight of u) - (weight of v) = w
    if (u == v) return;
    if (groups[ind[u]].size() > groups[ind[v]].size()) {
      std::swap(u, v);
      w = -w;
    }
    size_t pi=ind[u];
    // weight[u] <- weight[v]+w
    Weight x=weight[v]+w-weight[u];
    for (size_t i: groups[ind[u]]) {
      weight[i] += x;
      ind[i] = ind[v];
      groups[ind[v]].emplace_back(i);
    }
    groups[pi].clear();
  }

  Weight diff(size_t u, size_t v) const {
    if (ind[u] != ind[v])
      throw NotConnected();

    return weight[u]-weight[v];
  }

  void update(size_t u, size_t v, Weight w) {
    weight[u] += w;
    weight[v] += w;
    if (ind[u] != ind[v])
      unite(u, v, w);
  }

  void debug() const {
    fprintf(stderr, "Weight:\n");
    for (size_t i=0; i<weight.size(); ++i)
      fprintf(stderr, "%d%c", weight[i], i+1<weight.size()? ' ':'\n');

    fprintf(stderr, "Index:\n");
    for (size_t i=0; i<ind.size(); ++i)
      fprintf(stderr, "%zu%c", ind[i], i+1<ind.size()? ' ':'\n');

    fprintf(stderr, "groups:\n");
    for (size_t i=0; i<groups.size(); ++i) {
      fprintf(stderr, "{");
      for (size_t j=0; j<groups[i].size(); ++j) {
        fprintf(stderr, "%zu%s", groups[i][j], j+1<groups[i].size()? ", ":"");
      }
      fprintf(stderr, "}%s", i+1<groups.size()? ", ":"\n");
    }
    fprintf(stderr, "\n");
  }
};
