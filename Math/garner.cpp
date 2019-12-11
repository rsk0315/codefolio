template <typename Tp>
class simultaneous_congruences_garner {
public:
  using size_type = size_t;
  using value_type = Tp;

private:
  value_type M_mod = 1;
  value_type M_sol = 0;
  std::vector<value_type> M_c, M_m;

  static auto S_gcd_bezout(value_type a, value_type b) {
    value_type x{1}, y{0};
    for (value_type u{y}, v{x}; b != 0;) {
      value_type q{a/b};
      std::swap(x -= q*u, u);
      std::swap(y -= q*v, v);
      std::swap(a -= q*b, b);
    }
    return std::make_tuple(a, x, y);
  }

public:
  simultaneous_congruences_garner() = default;
  simultaneous_congruences_garner(simultaneous_congruences_garner const&) = default;
  simultaneous_congruences_garner(simultaneous_congruences_garner&&) = default;

  simultaneous_congruences_garner& operator =(simultaneous_congruences_garner const&) = default;
  simultaneous_congruences_garner& operator =(simultaneous_congruences_garner&&) = default;

  simultaneous_congruences_garner(std::initializer_list<std::pair<value_type, value_type>> il) {
    for (auto const& p: il) push(p.first, p.second);
  }

  void push(value_type a, value_type m) {
    if (M_c.empty()) {
      M_c.push_back(a);
      M_m.push_back(m);
      return;
    }

    value_type c = M_c[M_c.size()-1];
    value_type mi = M_m[M_m.size()-1];
    for (size_type i = M_c.size()-1; i--;) {
      c = (c * M_m[i] + M_c[i]) % m;
      (mi *= M_m[i]) %= m;
    }
    c = (a - c) * std::get<1>(S_gcd_bezout(mi, m)) % m;
    if (c < 0) c += m;
    M_c.push_back(c);
    M_m.push_back(m);
  }

  auto get(value_type m) const {
    value_type x = M_c[M_c.size()-1] % m;
    value_type y = M_c[M_c.size()-1] % m;
    for (size_type i = M_c.size()-1; i--;) {
      x = (x * M_m[i] + M_c[i]) % m;
      y = y * M_m[i] + M_c[i];
    }
    return x;
  }
};
