template <class RandomIt, class OutputIt>
OutputIt slide_min(RandomIt first, RandomIt last, ptrdiff_t width, OutputIt d_first) {
  std::deque<RandomIt> st;
  for (auto it = first; it < last; ++it) {
    while (!st.empty() && !(*st.back() < *it)) st.pop_back();
    st.push_back(it);
    if (width <= it - st.front()) st.pop_front();
    if (width-1 <= it - first) *d_first++ = *st.front();
  }
  return d_first;
}
