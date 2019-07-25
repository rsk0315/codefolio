template <typename RandomIt, typename OutputIt>
OutputIt window_minima(RandomIt first, RandomIt s_last, RandomIt last, OutputIt d_first) {
  std::deque<RandomIt> dq;
  ptrdiff_t width = std::distance(first, s_last);
  for (auto it = first; it < last; ++it) {
    while (!dq.empty() && *it < *dq.back()) dq.pop_back();
    dq.push_back(it);
    if (width <= it - dq.front()) dq.pop_front();
    if (width-1 <= it - first) *d_first++ = *dq.front();
  }
  return d_first;
}
