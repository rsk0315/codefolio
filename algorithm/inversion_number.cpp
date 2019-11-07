template <typename InputIt>
uintmax_t inversion(InputIt first, InputIt last) {
  using value_type = typename std::iterator_traits<InputIt>::value_type;
  std::vector<std::pair<value_type, size_t>> c;
  size_t n = 0;
  while (first != last) c.emplace_back(*first++, n++);
  std::sort(c.begin(), c.end());

  uintmax_t res = 0;
  basic_segment_tree<range_sum_single_add<uintmax_t>> st(n);
  for (size_t i = 0; i < n; ++i) {
    res += st.accumulate(c[i].second+1, n);
    st.modify(c[i].second, 1);
  }
  return res;
}
