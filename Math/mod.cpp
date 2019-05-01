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
Tp modadd(Tp a, Tp b, Tp mod) {
  a += b % mod;
  if (a < 0) a += mod;
  if (a >= mod) a -= mod;
  return a;
}

template <class Tp>
Tp modadd(const std::initializer_list<Tp>& adds, Tp mod) {
  Tp res = 0;
  for (const auto& add: adds) {
    res += add % mod;
    if (res < 0) res += mod;
    if (res >= mod) res -= mod;
  }
  return res;
}

template <class Tp>
Tp modsub(Tp a, Tp b, Tp mod) {
  a -= b % mod;
  if (a < 0) a += mod;
  if (a >= mod) a -= mod;
  return a;
}

template <class Tp>
Tp modmul(const std::initializer_list<Tp>& muls, Tp mod) {
  Tp res = 1;
  for (const auto& mul: muls) (res *= mul) %= mod;
  return res;
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
