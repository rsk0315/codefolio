std::vector<intmax_t> divisor(intmax_t n) {
  std::vector<intmax_t> res;
  for (intmax_t i = 1; i*i <= n; ++i) {
    if (n % i == 0) {
      res.push_back(i);
      if (i != n/i)
        res.push_back(n/i);
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}
