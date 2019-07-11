constexpr intmax_t mod = 998244353;

constexpr uintmax_t round_up_to_pow2(uintmax_t n) {
  if (!(n & (n-1))) return n;
  return static_cast<uintmax_t>(1) << ((CHAR_BIT * sizeof n) - __builtin_clzll(n));
}

constexpr uintmax_t lg2(uintmax_t n) {
  return (CHAR_BIT * sizeof n) - __builtin_clzll(n) - 1;
}

std::vector<intmax_t> fourier_transform(const std::vector<intmax_t>& f,
                                        intmax_t z, size_t n, intmax_t mod) {
  if (n == 1) return f;

  std::vector<intmax_t> f0(n/2), f1(n/2);
  for (size_t i = 0; i < n/2; ++i) {
    f0[i] = f[i<<1|0];
    f1[i] = f[i<<1|1];
  }

  intmax_t z2 = z * z % mod;
  f0 = fourier_transform(f0, z2, n/2, mod);
  f1 = fourier_transform(f1, z2, n/2, mod);

  std::vector<intmax_t> ff(n);
  intmax_t zp = 1;
  for (size_t i = 0; i < n/2; ++i) {
    ff[i] = (f0[i] + zp * f1[i]) % mod;
    zp = zp * z % mod;
  }
  for (size_t i = 0; i < n/2; ++i) {
    ff[i+n/2] = (f0[i] + zp * f1[i]) % mod;
    zp = zp * z % mod;
  }
  return ff;
}

int main() {
  size_t n;  // ...
  size_t m = round_up_to_pow2(n+n+2);
  std::vector<intmax_t> a(m), b(m);  // ...
  intmax_t p = 3;
  intmax_t u = 119;
  intmax_t zeta = modpow(p, u, mod);  // 2^23-th root of 2
  zeta = modpow(zeta, 1L<<(23-lg2(m)), mod);

  a = fourier_transform(a, zeta, m, mod);
  b = fourier_transform(b, zeta, m, mod);
  auto c = fourier_transform(a, modinv(zeta, mod), m, mod);
  for (auto& ci: c)
    (ci *= modinv(intmax_t(m), mod)) %= mod;
}
