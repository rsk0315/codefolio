template <class Tp>
std::vector<Tp> make_vector(size_t n, const Tp& x) {
  return std::vector<Tp>(n, x);
}

template <class... Ts>
auto make_vector(size_t n, size_t m, const Ts&... x) {
  return std::vector<decltype(make_vector(m, x...))>(n, make_vector(m, x...));
}
