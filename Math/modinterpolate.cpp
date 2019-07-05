intmax_t interpolate(std::vector<intmax_t> const& y, intmax_t x, intmax_t mod) {
  // based on Lagrange interpolation
  // y[i] = f(i) where i in [0, n]
  assert(0 <= x);

  intmax_t n = y.size()-1;
  if (x <= n) return y[x];

  modfactorial mf(n, mod);
  intmax_t omega = 1;
  for (intmax_t j = 0; j <= n; ++j)
    (omega *= (x-j)) %= mod;

  intmax_t sigma = 0;
  for (intmax_t i = 0; i <= n; ++i) {
    intmax_t wi = modmul({mf.inverse(i), mf.inverse(n-i)}, mod);
    if ((n-i) % 2 != 0) wi = mod-wi;
    (sigma += modmul({y[i], wi, modinv(x-i, mod)}, mod)) %= mod;
  }

  return modmul({sigma, omega}, mod);
}
