template <typename RandomIt, typename OutputIt, typename Compare = std::less<>>
OutputIt window_minima(RandomIt first, RandomIt s_last, RandomIt last,
                       OutputIt d_first, Compare comp = Compare()) {
  std::deque<RandomIt> dq;
  ptrdiff_t width = s_last - first;
  for (auto it = first; it < last; ++it) {
    while (!dq.empty() && comp(*it, *dq.back())) dq.pop_back();
    dq.push_back(it);
    if (width <= it - dq.front()) dq.pop_front();
    if (width-1 <= it - first) *d_first++ = *dq.front();
  }
  return d_first;
}
