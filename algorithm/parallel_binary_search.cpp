template <typename Oracle>
std::vector<size_t> parallel_binary_search(Oracle orc) {
  size_t m = orc.num_modifications();
  size_t q = orc.num_queries();
  std::vector<size_t> lb(q), ub(q, m+1);
  while (true) {
    bool decided = true;
    std::vector<std::vector<size_t>> ev(m+1);
    for (size_t j = 0; j < q; ++j) {
      if (ub[j]-lb[j] > 1) {
        decided = false;
        size_t mid = (lb[j]+ub[j]) >> 1;
        ev[mid].push_back(j);
      }
    }
    if (decided) break;

    orc.reset_state();
    for (size_t i = 0; i <= m; ++i) {
      while (!ev[i].empty()) {
        size_t j = ev[i].back();
        ev[i].pop_back();
        (orc.query(j)? ub[j]:lb[j]) = i;
      }
      if (i < m) orc.modify();
    }
  }
  return lb;
}
