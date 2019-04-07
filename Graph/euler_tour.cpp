template <class Tp>
std::array<std::vector<size_t>, 2> euler_tour(const graph<Tp>& g, size_t s = 0) {
  std::array<std::vector<size_t>, 2> res;
  res[0] = res[1] = std::vector<size_t>(g.size());
  size_t j = 0;
  std::function<void (size_t, size_t)> dfs = [&](size_t i, size_t pi) {
    res[0][i] = j++;
    for (const auto& e: g[i]) {
      if (e.dst == pi) continue;
      dfs(e.dst, i);
    }
    res[1][i] = j++;
  };
  dfs(s, -1);
  return res;
}
