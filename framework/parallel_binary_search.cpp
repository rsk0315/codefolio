template <typename StatefulPredicate, typename InputIt>
std::vector<size_t> parallel_partition_points(
    StatefulPredicate sp, InputIt first, InputIt last
) {
  if (first == last) return {};

  using value_type = typename std::iterator_traits<InputIt>::value_type;
  std::vector<value_type> qs(first, last);
  std::vector<size_t> lb(qs.size(), -1);
  std::vector<size_t> ub(qs.size(), sp.size()+1);

  bool completed = false;
  while (!completed) {
    completed = true;
    std::vector<std::vector<size_t>> ev(sp.size()+1);
    for (size_t j = 0; j < qs.size(); ++j) {
      if (ub[j]-lb[j] <= 1) continue;
      completed = false;
      size_t mid = (lb[j]+ub[j]) >> 1;
      ev[mid].push_back(j);
    }
    if (completed) break;

    sp.reset();
    for (size_t i = 0; i < ev.size(); ++i) {
      while (!ev[i].empty()) {
        size_t j = ev[i].back();
        ev[i].pop_back();
        (sp.predicate(qs[j])? lb[j]: ub[j]) = i;
      }
      if (i < sp.size()) sp.proceed();
    }
  }
  return ub;
}
