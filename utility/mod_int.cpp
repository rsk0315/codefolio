template <typename Tp, Tp Modulo>
class modint {
  // FIXME to implement with Montgomery multiplication

public:
  using value_type = typename std::make_signed<Tp>::type;

private:
  static constexpr value_type S_mod = Modulo;
  value_type M_value = 0;
  value_type M_mod;  // runtime value (used if S_mod == 0)

  static constexpr value_type S_inv(value_type n, value_type m) {
    value_type x = 0;
    value_type y = 1;
    value_type a = n;
    value_type b = m;
    for (value_type u = y, v = x; a;) {
      value_type q = b / a;
      std::swap(x -= q*u, u);
      std::swap(y -= q*v, v);
      std::swap(b -= q*a, a);
    }
    if ((x %= m) < 0) x += m;
    return x;
  }

  static value_type S_normalize(value_type n, value_type m) {
    if (n >= m) {
      n %= m;
    } else if (n < 0) {
      if ((n %= m) < 0) n += m;
    }
    return n;
  }

public:
  modint() = default;
  modint(modint const&) = default;
  modint(modint&&) = default;
  template <typename Up = Tp, typename std::enable_if<(Modulo > 0), Up>::type* = nullptr>
  modint(value_type n):
    M_value(S_normalize(n, Modulo)) {}
  template <typename Up = Tp, typename std::enable_if<(Modulo == 0), Up>::type* = nullptr>
  modint(value_type n, value_type m):
    M_value(S_normalize(n, m)), M_mod(m) {}
  // copying mod
  modint(modint const& base, value_type n):
    M_value(S_normalize(n, base.modulo())), M_mod(base.M_mod) {}

  modint& operator =(modint const&) = default;
  modint& operator =(modint&&) = default;
  modint& operator =(value_type n) {
    M_value = S_normalize(n, modulo());
    return *this;
  }

  modint& operator +=(modint const& other) {
    if ((M_value += other.M_value) >= modulo()) M_value -= modulo();
    return *this;
  }
  modint& operator -=(modint const& other) {
    if ((M_value -= other.M_value) < 0) M_value += modulo();
    return *this;
  }
  modint& operator *=(modint const& other) {
    (M_value *= other.M_value) %= modulo();
    return *this;
  }
  modint& operator /=(modint const& other) {
    (M_value *= S_inv(other.M_value, modulo())) %= modulo();
    return *this;
  }

  modint& operator +=(value_type const& n) {
    if ((M_value += S_normalize(n, modulo())) >= modulo()) M_value -= modulo();
    return *this;
  }
  modint& operator -=(value_type const& n) {
    if ((M_value -= S_normalize(n, modulo())) < 0) M_value += modulo();
    return *this;
  }
  modint& operator *=(value_type const& n) {
    (M_value *= S_normalize(n, modulo())) %= modulo();
    return *this;
  }
  modint& operator /=(value_type const& n) {
    (M_value *= S_inv(S_normalize(n, modulo()), modulo())) %= modulo();
    return *this;
  }

  modint operator +(modint const& other) const { return modint(*this) += other; }
  modint operator -(modint const& other) const { return modint(*this) -= other; }
  modint operator *(modint const& other) const { return modint(*this) *= other; }
  modint operator /(modint const& other) const { return modint(*this) /= other; }
  modint operator +(value_type const& n) const { return modint(*this) += n; }
  modint operator -(value_type const& n) const { return modint(*this) -= n; }
  modint operator *(value_type const& n) const { return modint(*this) *= n; }
  modint operator /(value_type const& n) const { return modint(*this) /= n; }

  modint operator +() const { return *this; }
  modint operator -() const {
    if (M_value == 0) return *this;
    return modint(*this, modulo()-M_value);
  }

  modint pow(intmax_t iexp) const {
    modint res(*this, 1);
    for (modint dbl = *this; iexp; iexp >>= 1) {
      if (iexp & 1) res *= dbl;
      dbl *= dbl;
    }
    return res;
  }
  modint& pow_eq(intmax_t iexp) { return *this = this->pow(iexp); }

  bool operator ==(modint const& other) const {
    return M_value == other.M_value;
  }
  bool operator ==(value_type const& n) const {
    return M_value == S_normalize(n, modulo());
  }
  bool operator !=(modint const& other) const { return !(*this == other); }
  bool operator !=(value_type const& n) const { return !(*this == n); }

  value_type get() const { return M_value; }
  value_type modulo() const { return ((S_mod > 0)? S_mod: M_mod); }

  static value_type generator(value_type p) {
    if (p == 998244353) return 3;
    return -1;
  }

  std::vector<modint> sqrt() const {
    if (*this == 0) return {*this};

    intmax_t const p = modulo();
    if (p % 4 == 3) {
      modint r = pow((p+1) / 4);
      if (r * r == *this) return {r, -r};
      return {};
    }

    value_type s = bit::ctz(p-1);
    value_type q = (p-1) >> s;

    modint z;
    for (value_type z0 = 2; z0 < p; ++z0) {
      z = modint(*this, z0);
      if (z.pow((p-1) / 2) == -1) break;
    }

    value_type m = s;
    modint c = z.pow(q);
    modint t = this->pow(q);
    modint r = this->pow((q+1) / 2);

    while (true) {
      if (t == 0) return {modint(*this, 0)};
      if (t == 1) return {r, -r};

      value_type i = 0;
      for (auto tt = t; tt != 1; ++i) tt *= tt;
      if (i == m) return {};
      auto b = c;
      for (value_type j = 0; j < m-i-1; ++j) b *= b;
      m = i;
      c = b * b;
      t *= c;
      r *= b;
    }
  }
};

template <typename T1, typename T2, T2 Modulo>
modint<T2, Modulo> operator +(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return rhs + lhs;
}
template <typename T1, typename T2, T2 Modulo>
modint<T2, Modulo> operator -(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return -(rhs - lhs);
}
template <typename T1, typename T2, T2 Modulo>
modint<T2, Modulo> operator *(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return rhs * lhs;
}
template <typename T1, typename T2, T2 Modulo>
modint<T2, Modulo> operator /(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return modint<T2, Modulo>(rhs, lhs) / rhs;
}
template <typename T1, typename T2, T2 Modulo>
bool operator ==(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return rhs == lhs;
}
template <typename T1, typename T2, T2 Modulo>
bool operator !=(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return !(lhs == rhs);
}

// constexpr intmax_t mod = 1'000'000'007;  // '
// constexpr intmax_t mod = 998244353;
using mi = modint<intmax_t, mod>;
