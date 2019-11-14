template <typename Monoid, typename UndirectedTree>
auto dp_on_tree(UndirectedTree const& g) {
  Monoid e{};
  size_t n = g.size();
  std::vector<size_t> parent(n, -1_zu);

  std::vector<std::vector<Monoid>> dp0(n), dp1(n);
  std::vector<Monoid> dp(n);
  for (size_t i = 0; i < n; ++i) {
    dp0[i].resize(g[i].size()+1, e);
    dp1[i].resize(g[i].size()+1, e);
  }

  make_fix_point([&](auto dfs0, size_t v, size_t p) -> Monoid {
      Monoid res = e;
      typename UndirectedTree::weight_type weight{};
      for (size_t i = 0; i < g[v].size(); ++i) {
        size_t u = g[v][i].target();
        if (u == p) {
          parent[v] = i;
          weight = g[v][i].weight();
          continue;
        }
        Monoid tmp = dfs0(u, v);
        res.append(tmp);
        dp0[v][i+1] = dp1[v][i] = tmp;
      }
      return res.f(weight);
  })(0, -1_zu);

  make_fix_point([&](auto dfs1, size_t v, size_t p, size_t pi) -> void {
      if (v != 0) {
        Monoid tmp = (dp0[p][pi] + dp1[p][pi+1]).f(g[p][pi].weight());
        dp0[v][parent[v]+1] = tmp;
        dp1[v][parent[v]] = tmp;
      }
      {
        for (size_t i = 1; i < dp0[v].size(); ++i)
          dp0[v][i].prepend(dp0[v][i-1]);
        for (size_t i = dp1[v].size()-1; i--;)
          dp1[v][i].append(dp1[v][i+1]);
        dp[v] = dp1[v][0];
      }
      for (size_t i = 0; i < g[v].size(); ++i) {
        size_t u = g[v][i].target();
        if (u != p) dfs1(u, v, i);
      }
  })(0, -1_zu, -1_zu);

  return dp;
}

class distance {
public:
  using value_type = int;

private:
  value_type M_value = 0;

public:
  distance() = default;
  distance(distance const&) = default;
  distance(distance&&) = default;
  distance(value_type v): M_value(v) {}

  distance& operator =(distance const&) = default;
  distance& operator =(distance&&) = default;

  distance& append(distance const& other) {
    M_value = std::max(other.M_value, M_value);
    return *this;
  }
  distance& prepend(distance const& other) {
    M_value = std::max(M_value, other.M_value);
    return *this;
  }
  distance operator +(distance const& other) const {
    return distance(*this).append(other);
  }
  template <typename Tp>
  distance f(Tp const& x) const { return distance(M_value + x); }
  value_type get() const { return M_value; }
};
