template <class BidirIt>
void radix_sort(BidirIt first, BidirIt last) {
  using value_type = typename BidirIt::value_type;
  constexpr int shift = 8;
  constexpr int max_exp = sizeof(value_type);
  constexpr size_t size = 1 << shift;
  constexpr size_t mask = size-1;
  std::vector<value_type> work(std::distance(first, last));
  for (int i = 0, cs = 0; i < max_exp; ++i) {
    size_t num[size] = {};

    for (BidirIt it = first; it != last; ++it)
      ++num[*it >> cs & mask];
    {
      size_t tmp = 0;
      for (size_t j = 0; j < size; ++j)
        std::swap(tmp, num[j] += tmp);
    }
    for (BidirIt it = first; it != last; ++it)
      work[num[*it >> cs & mask]++] = *it;
    {
      size_t j = 0;
      for (BidirIt it = first; it != last; ++it)
        *it = work[j++];
    }
    cs += shift;
  }
  if (!std::is_signed<value_type>::value) return;

  BidirIt mid = last;
  for (BidirIt it = last; it-- != first;)
    if (*it >= 0) {
      mid = ++it;
      break;
    }

  if (mid == last) return;
  ptrdiff_t len = std::distance(mid, last);
  std::copy(first, mid, std::next(work.begin(), len));
  std::copy(mid, last, work.begin());
  std::copy(work.begin(), work.end(), first);
}
