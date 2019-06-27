constexpr intmax_t operator ""_jd(unsigned long long n) {
  return static_cast<intmax_t>(n);
}

constexpr uintmax_t operator ""_ju(unsigned long long n) {
  return static_cast<uintmax_t>(n);
}

constexpr size_t operator ""_zu(unsigned long long n) {
  return static_cast<size_t>(n);
}

constexpr ptrdiff_t operator ""_td(unsigned long long n) {
  return static_cast<ptrdiff_t>(n);
}
