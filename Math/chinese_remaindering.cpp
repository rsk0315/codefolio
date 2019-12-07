template <typename Tp>
class simultaneous_congruences {
public:
  using value_type = Tp;

private:
  value_type M_mod = 1;
  value_type M_sol = 0;
  using safe_type = __int128;

  static auto S_gcd_bezout(value_type m, value_type n) {
    value_type x{0}, y{1};
    for (value_type u{y}, v{x}; m != 0;) {
      value_type q{n/m};
      std::swap(x -= q*u, u);
      std::swap(y -= q*v, v);
      std::swap(n -= q*m, m);
    }
    return std::make_tuple(n, x, y);
  }

public:
  simultaneous_congruences() = default;
  simultaneous_congruences(simultaneous_congruences const&) = default;
  simultaneous_congruences(simultaneous_congruences&&) = default;

  simultaneous_congruences& operator =(simultaneous_congruences const&) = default;
  simultaneous_congruences& operator =(simultaneous_congruences&&) = default;

  simultaneous_congruences(std::initializer_list<std::pair<value_type, value_type>> il) {
    for (auto const& p: il) push(p.first, p.second);
  }

  void push(value_type a, value_type m) {
    if (M_mod == 0) return;

    value_type g, x, y;
    std::tie(g, x, y) = S_gcd_bezout(M_mod, m);

    value_type mod = M_mod / g * m;
    value_type sol = (safe_type(M_mod) / g * a % mod * x + safe_type(m) / g * M_sol % mod * y) % mod;
    if (sol < 0) sol += mod;
    if (g > 1) {
      if (sol % M_mod != M_sol || sol % m != a) {
        M_mod = M_sol = 0;
        return;
      }
    }
    M_sol = sol;
    M_mod = mod;
  }

  auto get() const {
    return std::make_pair(M_sol, M_mod);
  }
};
