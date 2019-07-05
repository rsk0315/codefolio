template <typename Tp>
intmax_t ord(Tp g, Tp p, std::map<Tp, intmax_t> const& f) {
  intmax_t q = p-1;
  for (auto const& pp: f) {
    for (intmax_t i = 0; i < pp.second; ++i) {
      Tp e = pp.first;
      if (modpow(g, q/e, p) == 1)
        q /= e;
    }
  }
  return q;
}
