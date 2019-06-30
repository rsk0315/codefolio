template <class Tp>
intmax_t ord(Tp g, Tp p, const std::map<Tp, intmax_t>& f) {
  intmax_t q = p-1;
  for (const auto& pp: f) {
    for (intmax_t i = 0; i < pp.second; ++i) {
      Tp e = pp.first;
      if (modpow(g, q/e, p) == 1)
        q /= e;
    }
  }
  return q;
}
