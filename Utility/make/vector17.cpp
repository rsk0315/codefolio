template <size_t Dm, typename Tp>
auto make_vector(std::vector<size_t> sizes, const Tp& x) {
  size_t size = 0;
  if (!sizes.empty()) {
    size = sizes[0];
    sizes.erase(sizes.begin());
  }
  if constexpr (Dm > 1) {
    return std::vector<decltype(make_vector<Dm-1, Tp>(sizes, x))>(size, make_vector<Dm-1, Tp>(sizes, x));
  } else {
    return std::vector<Tp>(x);
  }
}
