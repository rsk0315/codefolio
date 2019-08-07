template <class BidirIt>
void radix_sort(BidirIt first, BidirIt last) {
  using value_type = typename BidirIt::value_type;
  intmax_t cur_shift = 0;
  intmax_t max_exp = sizeof(value_type);
  std::vector<value_type> work(std::distance(first, last));
  for (int i = 0; i < max_exp; ++i) {
    size_t num[256] = {};

    for (BidirIt it = first; it != last; ++it)
      ++num[*it >> cur_shift & 255];
    {
      size_t tmp = 0;
      for (int j = 0; j < 256; ++j)
        std::swap(tmp, num[j] += tmp);
    }
    for (BidirIt it = first; it != last; ++it)
      work[num[*it >> cur_shift & 255]++] = *it;
    {
      size_t j = 0;
      for (BidirIt it = first; it != last; ++it)
        *it = work[j++];
    }
    cur_shift += 8;
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
