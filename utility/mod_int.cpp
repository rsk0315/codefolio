template <typename Tp, Tp modulo>
class modint {
  // FIXME to implement with Montgomery multiplication

public:
  using value_type = typename std::make_signed<Tp>::type;

private:
  static constexpr value_type S_mod = modulo;
  value_type M_value = 0;
  value_type M_mod;  // runtime value (used if S_mod == 0)

  static constexpr value_type S_normalize(value_type n) {
    if ((n %= S_mod) < 0) n += S_mod;
    return n;
  }

  static constexpr value_type S_normalize(value_type n, value_type m) {
    if ((n %= m) < 0) n += m;
    return n;
  }

  static constexpr value_type S_gcd(
      value_type a, value_type b, value_type& x, value_type& y
  ) {
    x = 0;
    y = 1;
    for (value_type u = y, v = x; a;) {
      value_type q = b / a;
      std::swap(x -= q*u, u);
      std::swap(y -= q*v, v);
      std::swap(b -= q*a, a);
    }
    return b;
  }

  static constexpr value_type S_inv(value_type n) {
    value_type x, y;
    S_gcd(n, S_mod, x, y);
    if ((x %= S_mod) < 0) x += S_mod;
    return x;
  }

  static constexpr value_type S_inv(value_type n, value_type m) {
    value_type x, y;
    S_gcd(n, m, x, y);
    if ((x %= m) < 0) x += m;
    return x;
  }

public:
  modint() = default;
  modint(modint const&) = default;
  modint(modint&&) = default;
  modint(value_type n): M_value(S_normalize(n)) {
    assert(S_mod > 0);
  }
  modint(value_type n, value_type m): M_value(S_normalize(n, m)), M_mod(m) {
    assert(S_mod == 0);
    assert(m > 0);
  }

  modint& operator =(modint const&) = default;
  modint& operator =(modint&&) = default;
  modint& operator =(value_type n) {
    assert(S_mod > 0);
    M_value = S_normalize(n, S_mod);
    return *this;
  }

  modint& operator +=(modint const& other) {
    if (S_mod > 0) {
      if ((M_value += other.M_value) >= S_mod) M_value -= S_mod;
    } else {
      assert(M_mod == other.M_mod);
      if ((M_value += other.M_value) >= M_mod) M_value -= M_mod;
    }
    return *this;
  }
  modint& operator -=(modint const& other) {
    if (S_mod > 0) {
      if ((M_value -= other.M_value) < 0) M_value += S_mod;
    } else {
      assert(M_mod == other.M_mod);
      if ((M_value -= other.M_value) < 0) M_value += M_mod;
    }
    return *this;
  }
  modint& operator *=(modint const& other) {
    if (S_mod > 0) {
      (M_value *= other.M_value) %= S_mod;
    } else {
      assert(M_mod == other.M_mod);
      (M_value *= other.M_value) %= M_mod;
    }
    return *this;
  }
  modint& operator /=(modint const& other) {
    if (S_mod > 0) {
      (M_value *= S_inv(other.M_value)) %= S_mod;
    } else {
      assert(M_mod == other.M_mod);
      (M_value *= S_inv(other.M_value, M_mod)) %= M_mod;
    }
    return *this;
  }

  modint& operator +=(value_type const& n) {
    if (S_mod > 0) return (*this += modint(n));
    return (*this += modint(n, M_mod));
  }
  modint& operator -=(value_type const& n) {
    if (S_mod > 0) return (*this -= modint(n));
    return (*this -= modint(n, M_mod));
  }
  modint& operator *=(value_type const& n) {
    if (S_mod > 0) return (*this *= modint(n));
    return (*this *= modint(n, M_mod));
  }
  modint& operator /=(value_type const& n) {
    if (S_mod > 0) return (*this /= modint(n));
    return (*this /= modint(n, M_mod));
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
    if (S_mod > 0) return modint(0) -= *this;
    return modint(0, M_mod) -= *this;
  }

  modint pow(intmax_t iexp) const {
    modint res = ((S_mod > 0)? modint(1): modint(1, M_mod));
    for (modint dbl = *this; iexp; iexp >>= 1) {
      if (iexp & 1) res *= dbl;
      dbl *= dbl;
    }
    return res;
  }
  modint& pow_eq(intmax_t iexp) { return *this = this->pow(iexp); }

  value_type get() const { return M_value; }
};
