template <typename ForwardIt, typename Hash = std::hash<typename ForwardIt::value_type>>
void bucket_sort(ForwardIt first, ForwardIt last, Hash hf = Hash()) {
  using value_type = typename ForwardIt::value_type;
  size_t size = 0;
  for (auto it = first; it != last; ++it)
    size = std::max(size, hf(*it)+1);

  std::vector<std::queue<value_type>> bucket(size);
  for (auto it = first; it != last; ++it) {
    size_t i = hf(*it);
    bucket[i].push(std::move(*it));
  }

  for (size_t i = 0; i < size; ++i)
    while (!bucket[i].empty()) {
      *first++ = std::move(bucket[i].front());
      bucket[i].pop();
    }
}
