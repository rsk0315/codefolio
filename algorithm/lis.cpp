template <typename InputIt, typename OutputIt>
OutputIt longest_increasing(InputIt first, InputIt last, OutputIt d_first) {
  using value_type = typename InputIt::value_type;
  using difference_type = typename InputIt::difference_type;
  value_type inf = std::numeric_limits<value_type>::max();
  difference_type len = std::distance(first, last);
  std::vector<value_type> dp(len+1, inf);
  do {
    auto it = std::lower_bound(dp.begin(), dp.end(), *first);
    if (!(*it < inf)) *d_first++ = *first;
    *it = *first;
  } while (++first != last);
  return d_first;
}

template <typename InputIt>
typename InputIt::difference_type longest_increasing(InputIt first, InputIt last) {
  using value_type = typename InputIt::value_type;
  using difference_type = typename InputIt::difference_type;
  value_type inf = std::numeric_limits<value_type>::max();
  difference_type len = std::distance(first, last);
  std::vector<value_type> dp(len+1, inf);
  difference_type res = 0;
  do {
    auto it = std::lower_bound(dp.begin(), dp.end(), *first);
    if (!(*it < inf)) ++res;
    *it = *first;
  } while (++first != last);
  return res;
}
