#warning "Newer version is available: /algorithm/radix_sort.cpp"

template <class RandomIt>
auto rsort(RandomIt first, RandomIt last)
  -> typename std::enable_if<std::is_integral<
    typename RandomIt::value_type>::value>::type
{
  using IntType = typename RandomIt::value_type;
  intmax_t cur_shift=0, max_exp=sizeof(IntType);
  std::vector<IntType> work(last-first);
  for (int i=0; i<max_exp; ++i) {
    size_t num[256]={};

    for (RandomIt it=first; it<last; ++it)
      ++num[((*it)>>cur_shift)&255];

    {
      size_t tmp=0;
      for (int j=0; j<256; ++j)
        std::swap(tmp, num[j]+=tmp);
    }

    for (RandomIt it=first; it<last; ++it)
      work[num[((*it)>>cur_shift)&255]++] = *it;

    {
      size_t j=0;
      for (RandomIt it=first; it<last; ++it)
        *it = work[j++];
    }

    cur_shift += 8;
  }

  if (!std::is_signed<IntType>::value) return;

  RandomIt mid=last;
  for (RandomIt it=last; --it>=first;)
    if (*it >= 0) {
      mid = ++it;
      break;
    }

  if (mid == last)
    return;

  std::copy(first, mid, work.begin()+(last-mid));
  std::copy(mid, last, work.begin());
  std::copy(work.begin(), work.end(), first);
}

template <class RandomIt>
auto rsort(RandomIt first, RandomIt last)
  -> typename std::enable_if<std::is_integral<
    typename RandomIt::value_type::first_type>::value>::type
{
  using PairType = typename RandomIt::value_type;
  using KeyType = typename PairType::first_type;

  intmax_t cur_shift=0, max_exp=sizeof(KeyType);
  std::vector<PairType> work(last-first);
  for (int i=0; i<max_exp; ++i) {
    size_t num[256]={};

    for (RandomIt it=first; it<last; ++it)
      ++num[((it->first)>>cur_shift)&255];

    {
      size_t tmp=0;
      for (int j=0; j<256; ++j)
        std::swap(tmp, num[j]+=tmp);
    }

    for (RandomIt it=first; it<last; ++it)
      work[num[((it->first)>>cur_shift)&255]++] = *it;

    {
      size_t j=0;
      for (RandomIt it=first; it<last; ++it)
        *it = work[j++];
    }

    cur_shift += 8;
  }

  if (!std::is_signed<KeyType>::value) return;

  RandomIt mid=last;
  for (RandomIt it=last; --it>=first;)
    if ((it->first) >= 0) {
      mid = ++it;
      break;
    }

  if (mid == last)
    return;

  std::copy(first, mid, work.begin()+(last-mid));
  std::copy(mid, last, work.begin());
  std::copy(work.begin(), work.end(), first);
}
