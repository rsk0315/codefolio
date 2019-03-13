std::map<intmax_t, intmax_t> factor(intmax_t n) {
  std::map<intmax_t, intmax_t> res;
  for (intmax_t i = 2; i*i <= n; ++i) {
    while (n % i == 0) {
      ++res[i];
      n /= i;
    }
  }
  if (n > 1) ++res[n];
  return res;
}
