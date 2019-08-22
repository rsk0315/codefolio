class tree_lowest_common_ancestor {
public:
  using size_type = size_t;

private:
  std::vector<size_type> M_e;
  sparse_table<std::pair<size_type, size_type>> M_st;

  template <typename Graph>
  void M_dfs(Graph const& g, size_type v, size_type p, size_type d0,
             std::vector<std::pair<size_type, size_type>>& d) {

    M_e[v] = d.size();
    d.emplace_back(d0, v);
    for (auto const& e: g[v]) {
      if (e.target() == p) continue;
      M_dfs(g, e.target(), v, d0+1, d);
      d.emplace_back(d0, v);
    }
  }

public:
  tree_lowest_common_ancestor() = default;
  tree_lowest_common_ancestor(tree_lowest_common_ancestor const&) = default;
  tree_lowest_common_ancestor(tree_lowest_common_ancestor&&) = default;

  template <typename Graph>
  tree_lowest_common_ancestor(Graph const& g, size_type r): M_e(g.size()) {
    std::vector<std::pair<size_type, size_type>> d;
    M_dfs(g, r, -1, 0, d);
    M_st = sparse_table<std::pair<size_type, size_type>>(d.begin(), d.end());
  }

  size_type operator ()(size_type u, size_type v) const {
    size_type x, y;
    std::tie(x, y) = std::minmax(M_e[u], M_e[v]);
    return M_st.get(x, y+1).second;
  }
};
