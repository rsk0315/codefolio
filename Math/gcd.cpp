template <typename Tp>
Tp gcd(Tp m, Tp n) {
  while (n != Tp()) std::swap(m %= n, n);
  return m;
}

template <typename Tp>
Tp gcd(Tp a, Tp b, Tp& x, Tp& y) {
  x = Tp(0);
  y = Tp(1);
  for (Tp u = y, v = x; a;) {
    Tp q = b/a;
    std::swap(x -= q*u, u);
    std::swap(y -= q*v, v);
    std::swap(b -= q*a, a);
  }
  return b;
}
