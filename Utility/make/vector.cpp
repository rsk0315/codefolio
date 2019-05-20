template <typename Tp, size_t N>
std::vector<Tp> make_vector_detail(std::vector<size_t> sizes, typename std::enable_if<(N == 1), const Tp&>::type x) {
  return std::vector<Tp>(sizes[0], x);
}
template <typename Tp, size_t N>
auto make_vector_detail(std::vector<size_t> sizes, typename std::enable_if<(N > 1), const Tp&>::type x) {
  size_t size = sizes[N-1];
  sizes.pop_back();
  return std::vector<decltype(make_vector_detail<Tp, N-1>(sizes, x))>(size, make_vector_detail<Tp, N-1>(sizes, x));
}

template <typename Tp, size_t N>
auto make_vector(const size_t(&sizes)[N], const Tp& x) {
  std::vector<size_t> s(N);
  for (size_t i = 0; i < N; ++i) s[i] = sizes[N-i-1];
  return make_vector_detail<Tp, N>(s, x);
}
