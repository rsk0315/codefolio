template <typename Tp, size_t N>
auto make_vector_detail(std::vector<size_t>& sizes, const Tp& x) {
  size_t size = sizes[N-1];
  if constexpr (N > 1) {
    sizes.pop_back();
    return std::vector<decltype(make_vector_detail<Tp, N-1>(sizes, x))>(size, make_vector_detail<Tp, N-1>(sizes, x));
  } else {
    return std::vector<Tp>(size, x);
  }
}

template <typename Tp, size_t N>
auto make_vector(const size_t(&sizes)[N], const Tp& x) {
  std::vector<size_t> s(N);
  for (size_t i = 0; i < N; ++i) s[i] = sizes[N-i-1];
  return make_vector_detail<Tp, N>(s, x);
}
