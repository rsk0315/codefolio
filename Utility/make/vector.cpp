template <size_t Dm, typename Tp>
std::vector<Tp> make_vector(std::vector<size_t> sizes, typename std::enable_if<(Dm == 1), const Tp&>::type x) {
  size_t size = 0;
  if (!sizes.empty()) size = sizes[0];
  return std::vector<Tp>(size, x);
}

template <size_t Dm, typename Tp>
auto make_vector(std::vector<size_t> sizes, typename std::enable_if<(Dm > 1), const Tp&>::type x) {
  size_t size = 0;
  if (!sizes.empty()) {
    size = sizes[0];
    sizes.erase(sizes.begin());
  }
  return std::vector<decltype(make_vector<Dm-1, Tp>(sizes, x))>(size, make_vector<Dm-1, Tp>(sizes, x));
}
