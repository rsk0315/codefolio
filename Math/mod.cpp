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
Tp modsub(Tp a, Tp b, Tp mod) {
  a -= b % mod;
  if (a < 0) a += mod;
  if (a >= mod) a -= mod;
  return a;
}

template <class Tp>
class congruences {
  Tp sol, mod=1;

public:
  congruences() {}

  std::pair<Tp, Tp> append(Tp a, Tp q) {
    if (mod == 1) {
      sol = a % (mod = q);
    } else {
      Tp x = (modsub(a, sol, q) * modinv(mod, q)) % q;
      sol += x * mod;
      mod *= q;
    }
    return {sol, mod};
  }

  std::pair<Tp, Tp> get() const {
    return {sol, mod};
  }
};
