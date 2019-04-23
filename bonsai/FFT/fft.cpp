#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>

template <class Tp>
Tp gcd(Tp a, Tp b, Tp& x, Tp& y) {
  x = Tp(0);
  y = Tp(1);
  for (Tp u=y, v=x; a;) {
    Tp q = b/a;
    std::swap(x-=q*u, u);
    std::swap(y-=q*v, v);
    std::swap(b-=q*a, a);
  }
  return b;
}

template <class Tp>
Tp modinv(Tp a, Tp mod) {
  Tp x, y;
  gcd(a, mod, x, y);
  x %= mod;
  if (x < 0) x += mod;
  return x;
}

template <class Tp>
Tp moddiv(Tp n, Tp d, Tp mod) {
  return n * modinv(d, mod) % mod;
}

intmax_t pow_two_at_least(intmax_t n) {
  if ((n & (n-1)) == 0) return n;
  return intmax_t(2) << (63 - __builtin_clzll(n));
}

std::vector<intmax_t> fourier_transform(const std::vector<intmax_t>& f, size_t n) {
  if (n == 1) return f;

  std::vector<intmax_t> f0(n/2), f1(n/2);
  for (size_t i = 0; i < n/2; ++i) {
    f0[i] = f[2*i+0];
    f1[i] = f[2*i+1];
  }

  auto ff0 = fourier_transform(f0, n/2);
  auto ff1 = fourier_transform(f1, n/2);
  intmax_t mod = 998244353;
  intmax_t zeta = 15311432;
  intmax_t pow_zeta = 1;
  std::vector<intmax_t> ff(n);
  for (size_t i = 0; i < n; ++i) {
    ff[i] = (ff0[i % (n/2)] + pow_zeta*ff1[i % (n/2)]) % mod;
    pow_zeta = pow_zeta * zeta % mod;
  }
  return ff;
}

std::vector<intmax_t> inverse_fourier_transform(const std::vector<intmax_t>& f, size_t n) {
  if (n == 1) return f;

  std::vector<intmax_t> f0(n/2), f1(n/2);
  for (size_t i = 0; i < n/2; ++i) {
    f0[i] = f[2*i+0];
    f1[i] = f[2*i+1];
  }

  auto ff0 = inverse_fourier_transform(f0, n/2);
  auto ff1 = inverse_fourier_transform(f1, n/2);
  intmax_t mod = 998244353;
  intmax_t zeta = modinv(intmax_t(15311432), mod);
  intmax_t pow_zeta = 1;
  std::vector<intmax_t> ff(n);
  for (size_t i = 0; i < n; ++i) {
    ff[i] = (ff0[i % (n/2)] + pow_zeta*ff1[i % (n/2)]) % mod;
    pow_zeta = pow_zeta * zeta % mod;
  }
  return ff;
}

template <class Tp>
Tp modpow(Tp base, intmax_t iexp, Tp mod) {
  Tp res = 1;
  for (Tp dbl = base; iexp; iexp >>= 1) {
    if (iexp & 1) res = res * dbl % mod;
    dbl = dbl * dbl % mod;
  }
  return res;
}

intmax_t primitive_root(intmax_t p) {
  intmax_t phi = p-1;
  std::vector<intmax_t> factor;
  intmax_t n = p;
  for (intmax_t i = 2; i*i <= n; ++i)
    if (n % i == 0) {
      factor.push_back(i);
      do n /= i; while (n % i == 0);
    }
  if (n > 1) factor.push_back(n);

  for (intmax_t res = 2; res <= p; ++res) {
    bool ok = true;
    for (auto fi: factor) {
      if (modpow(res, phi/fi, p) != 1) {
        ok = false;
        break;
      }
    }
    if (ok) return res;
  }
  assert(false);
}

int main() {
  size_t n;
  scanf("%zu", &n);

  size_t m = pow_two_at_least(n+n-1);
  std::vector<intmax_t> a(m), b(m);
  for (size_t i = 1; i <= n; ++i)
    scanf("%jd %jd", &a[i], &b[i]);

  auto fa = fourier_transform(a, m);
  auto fb = fourier_transform(b, m);

  intmax_t mod = 998244353;
  std::vector<intmax_t> fc(m);
  for (size_t i = 0; i < m; ++i)
    fc[i] = (fa[i] * fb[i]) % mod;

  auto c = inverse_fourier_transform(fc, m);
  for (auto& ci: c) ci = moddiv(ci, intmax_t(m), mod);

  for (auto ci: c) printf("%jd\n", ci);
}
