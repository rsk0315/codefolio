template <typename ForwardIt>
void forward_merge(ForwardIt first, ForwardIt mid, ForwardIt last) {
  using value_type = typename std::iterator_traits<ForwardIt>::value_type;
  std::vector<value_type> left(std::make_move_iterator(first), std::make_move_iterator(mid));
  std::vector<value_type> right(std::make_move_iterator(mid), std::make_move_iterator(last));
  auto itl = left.begin();
  auto itr = right.begin();
  while (first != last) {
    bool from_left = !(itl == left.end() || (itr != right.end() && *itr < *itl));
    *first++ = std::move(*(from_left? itl: itr));
    ++(from_left? itl: itr);
  }
}

template <typename ForwardIt>
void forward_sort(ForwardIt first, ForwardIt last) {
  if (first == last) return;

  using difference_type = typename std::iterator_traits<ForwardIt>::difference_type;
  auto next_until = [](ForwardIt it, difference_type n, ForwardIt sent) {
    while (n-- && it != sent) ++it;
    return it;
  };
  for (difference_type width = 1; true; width <<= 1) {
    auto first0 = first;
    while (first0 != last) {
      auto mid0 = next_until(first0, width, last);
      auto last0 = next_until(mid0, width, last);
      if (mid0 == last0) {
        if (first == first0) return;
        break;
      }
      my_merge(first0, mid0, last0);
      first0 = last0;
    }
  }
}
