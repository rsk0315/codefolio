class quick_find {
  std::vector<std::vector<size_t>> groups;
  std::vector<size_t> index;

public:
  quick_find(size_t n): index(n) {
    std::iota(index.begin(), index.end(), 0);
    for (size_t i = 0; i < n; ++i)
      groups.emplace_back(1, i);
  }

  bool unite(size_t u, size_t v) {
    if (index[u] == index[v]) return false;

    if (groups[index[u]].size() > groups[index[v]].size())
      std::swap(u, v);

    size_t pi = index[u];
    for (size_t i: groups[index[u]]) {
      index[i] = index[v];
      groups[index[v]].push_back(i);
    }
    groups[pi].clear();

    return true;
  }

  const std::vector<size_t>& components(size_t v) const {
    return groups[index[v]];
  }

  bool connected(size_t u, size_t v) const {
    return index[u] == index[v];
  }
};
