template <typename Tp, Tp Modulo>
class modtable {
public:
  using size_type = size_t;
  using underlying_type = typename std::make_signed<Tp>::type;
  using value_type = modint<Tp, Modulo>;

private:
  std::vector<value_type> M_f, M_fi;

public:
  modtable() = default;
  modtable(modtable const&) = default;
  modtable(modtable&&) = default;

  modtable(underlying_type n): M_f(n+1), M_fi(n+1) {
    assert(Modulo > 0);
    M_f[0] = 1;
    for (underlying_type i = 1; i <= n; ++i) M_f[i] = M_f[i-1] * i;
    M_fi[n] = value_type(1) / M_f[n];
    for (underlying_type i = n; i > 0; --i) M_fi[i-1] = M_fi[i] * i;
  }

  modtable(underlying_type n, underlying_type m): M_f(n+1), M_fi(n+1) {
    assert(Modulo == 0);
    M_f[0] = underlying_type(1, m);
    for (underlying_type i = 1; i <= n; ++i) M_f[i] = M_f[i-1] * i;
    M_fi[n] = underlying_type(1, m) / M_f[n];
    for (underlying_type i = n; i > 0; --i) M_fi[i-1] = M_fi[i] * i;    
  }

  modtable& operator =(modtable const&) = default;
  modtable& operator =(modtable&&) = default;

  value_type factorial(underlying_type n) const { return M_f[n]; }
  value_type inversed_factorial(underlying_type n) const { return M_fi[n]; }
  value_type choose(underlying_type n, underlying_type r) const {
    // XXX implicitly assuming n < Modulo
    if (!(0 <= n && 0 <= r && r <= n)) {
      if (Modulo == 0) return value_type(0, M_f[0].modulo());
      return value_type(0);
    }
    return M_f[n] * M_fi[r] * M_fi[n-r];
  }
};
