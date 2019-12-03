#include <cstdio>
#include <cstdint>
#include <cassert>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <stack>

using namespace std::literals;

constexpr intmax_t  operator ""_jd(unsigned long long n) { return n; }
constexpr uintmax_t operator ""_ju(unsigned long long n) { return n; }
constexpr size_t    operator ""_zu(unsigned long long n) { return n; }
// constexpr ptrdiff_t operator ""_td(unsigned long long n) { return n; }

namespace bit {
template <typename Tp>
int log2p1(Tp n) {
  if (n == 0) return 0;
  return (CHAR_BIT * sizeof(long long)) - __builtin_clzll(n);
}

template <typename Tp>
Tp ceil2(Tp n) {
  if (n == 0) return 1;
  return Tp{1} << log2p1(n-1);
}

template <typename Tp>
int ctz(Tp n) {
  return __builtin_ctzll(n);
}
}  // bit::

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
    return S_normalize(x, m);
  }

  static value_type S_normalize(value_type n, value_type m) {
    if (n >= m) {
      n %= m;
    } else if (n < 0) {
      n %= m;
      n += m;
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
  modint(value_type n, modint const& base):
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
    return modint(modulo()-M_value, *this);
  }

  modint pow(intmax_t iexp) const {
    modint res(1, *this);
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
      z = modint(z0, *this);
      if (z.pow((p-1) / 2) == -1) break;
    }

    value_type m = s;
    modint c = z.pow(q);
    modint t = this->pow(q);
    modint r = this->pow((q+1) / 2);

    while (true) {
      if (t == 0) return {modint(0, *this)};
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
  return modint<T2, Modulo>(lhs, rhs) / rhs;
}
template <typename T1, typename T2, T2 Modulo>
bool operator ==(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return rhs == lhs;
}
template <typename T1, typename T2, T2 Modulo>
bool operator !=(T1 const& lhs, modint<T2, Modulo> const& rhs) {
  return !(lhs == rhs);
}

constexpr intmax_t mod = 998244353;
using mi = modint<intmax_t, mod>;

template <typename ModInt>
class mod_polynomial {
public:
  using value_type = ModInt;
  using size_type = size_t;

private:
  using underlying_type = typename value_type::value_type;
  std::vector<value_type> M_c;

  static std::vector<value_type> S_fft(std::vector<value_type>& f, value_type z) {
    size_type n = f.size();
    if (n == 1) return f;

    std::vector<value_type> f0(n/2), f1(n/2);
    for (size_type i = 0; i < n/2; ++i) {
      f0[i] = f[i<<1|0];
      f1[i] = f[i<<1|1];
    }

    value_type z2 = z * z;
    f0 = S_fft(f0, z2);
    f1 = S_fft(f1, z2);

    std::vector<value_type> ff(n);
    value_type zp(1, z);
    for (size_type i = 0; i < n/2; ++i) {
      ff[i] = (f0[i] + zp * f1[i]);
      zp *= z;
    }
    for (size_type i = 0; i < n/2; ++i) {
      ff[i+n/2] = (f0[i] + zp * f1[i]);
      zp *= z;
    }
    return ff;
  }

  static std::vector<value_type> S_normalize(std::vector<value_type>& c) {
    while (c.size() > 1 && c.back() == 0) c.pop_back();
    return c;
  }

  static std::vector<value_type> S_shift_left(std::vector<value_type> const& c,
                                              size_type k) {
    auto d = c;
    std::vector<value_type> tmp(k, value_type(0, d[0]));
    d.insert(d.begin(), tmp.begin(), tmp.end());
    return d;
  }

  static std::vector<value_type> S_shift_right(std::vector<value_type> const& c,
                                               size_type k) {
    if (c.size() <= k) return {value_type(0, c[0])};
    return std::vector<value_type>(c.begin()+k, c.end());
  }

  static std::vector<value_type> S_add(std::vector<value_type> const& a,
                                       std::vector<value_type> const& b) {
    std::vector<value_type> res = a;
    if (a.size() < b.size()) res.resize(b.size(), value_type(0, a[0]));
    for (size_type i = 0; i < b.size(); ++i)
      res[i] += b[i];
    return S_normalize(res);
  }

  static std::vector<value_type> S_subtract(std::vector<value_type> const& a,
                                            std::vector<value_type> const& b) {
    std::vector<value_type> res = a;
    if (a.size() < b.size()) res.resize(b.size(), value_type(0, a[0]));
    for (size_type i = 0; i < b.size(); ++i)
      res[i] -= b[i];
    return S_normalize(res);
  }

  static std::vector<value_type> S_multiply(std::vector<value_type> const& a,
                                            std::vector<value_type> const& b) {
    // O(n log n)

    assert(!a.empty() && !b.empty());
    size_type n = bit::ceil2(std::max(a.size(), b.size())) * 2;

    // p = u 2^e + 1
    underlying_type p = a[0].modulo();
    underlying_type e = bit::ctz(p-1);
    underlying_type u = (p >> e);
    assert((1_zu << e) >= n);

    value_type g(value_type::generator(p), a[0]);
    value_type z = g.pow(u);
    z.pow_eq(1_jd << (e - bit::ctz(n)));

    auto pa = a;
    auto pb = b;
    pa.resize(n, value_type(0, a[0]));
    pb.resize(n, value_type(0, a[0]));
    pa = S_fft(pa, z);
    pb = S_fft(pb, z);
    for (size_type i = 0; i < n; ++i) pa[i] *= pb[i];
    pa = S_fft(pa, 1/z);
    for (size_type i = 0; i < n; ++i) pa[i] /= n;
    return S_normalize(pa);
  }

  static std::vector<value_type> S_reciprocal(std::vector<value_type> const& a) {
    // a is of degree k = 2^l - 1
    size_type k = a.size();
    if (a.size() == 1) return {1/a[0]};
    
    std::vector<value_type> q = S_reciprocal(S_shift_right(a, k/2));
    std::vector<value_type> r = q;
    {
      // r <- (q*2) << (k/2*3-2)
      for (auto& c: r) c *= 2;
      r = S_shift_left(r, k/2*3-2);
    }
    {
      // r <- r - q^2 a[:k/2]
      std::vector<value_type> tmp = q;
      tmp = S_multiply(tmp, q);
      tmp = S_multiply(tmp, a);
      r = S_subtract(r, tmp);
    }
    return S_shift_right(r, k-2);  // r >> (k-2)
  }

  // https://stackoverflow.com/questions/44770632/
  // http://web.cs.iastate.edu/~cs577/handouts/polydivide.pdf
  static std::vector<value_type> S_quotient(std::vector<value_type> const& u,
                                            std::vector<value_type> const& v) {
    size_type n = v.size();
    std::vector<value_type> res{value_type(0, u[0])};
    size_type shift = bit::ceil2(n) - n;  // or more?
    auto u1 = S_shift_left(u, shift);
    auto v1 = S_shift_left(v, shift);
    n += shift-1;
    auto s = S_reciprocal(v1);
    auto t = S_multiply(s, v1);
    t.pop_back();
    t = S_normalize(t);
    for (auto& c: t) c = -c;
    while (u1.size() >= v1.size()) {
      res = S_add(res, S_shift_right(S_multiply(s, u1), n+n));
      u1 = S_shift_right(S_multiply(t, u1), n+n);
    }
    return res;
  }

  static std::vector<value_type> S_modulo(std::vector<value_type> const& u,
                                          std::vector<value_type> const& v) {
    // fprintf(stderr, "mod of size %zu * %zu\n", u.size(), v.size());
    return S_subtract(u, S_multiply(S_quotient(u, v), v));
  }

  static void S_inspect(std::vector<value_type> const& f, std::string const& s = "f(x)"s) {
    fprintf(stderr, "%s: ", s.c_str());
    for (size_type i = 0; i < f.size(); ++i) {
      intmax_t c = f[i].get();
      if (c > f[i].modulo()/2) c -= f[i].modulo();
      fprintf(stderr, "%jd%c", c, i+1<f.size()? ' ': '\n');
    }
  }
 
  std::vector<value_type> S_multipoint_evaluation(std::vector<value_type> const& xs) const {
    // O(n (log n)^2)

    size_type m = xs.size();
    std::vector<std::vector<value_type>> mul(m+m);
    for (size_type i = 0; i < m; ++i)
      mul[m+i] = {-xs[i], 1};
    for (size_type i = m; i-- > 1;) {
      mul[i] = S_multiply(mul[i<<1|0], mul[i<<1|1]);
      // fprintf(stderr, "[%zu] * [%zu], of size %zu\n", (i<<1|0), (i<<1|1), mul[i].size());
    }

    std::vector<std::vector<value_type>> fm(m+m);
    size_type l = m;
    size_type r = l+m;
    while (l < r) {
      if (l & 1) { fm[l] = S_modulo(M_c, mul[l]); ++l; }
      if (r & 1) { --r; fm[r] = S_modulo(M_c, mul[r]); }
      l >>= 1;
      r >>= 1;
    }

    for (size_type i = 0; i < m; ++i) {
      if (fm[i].empty()) continue;
      fm[i<<1|0] = S_modulo(fm[i], mul[i<<1|0]);
      fm[i<<1|1] = S_modulo(fm[i], mul[i<<1|1]);
    }

    std::vector<value_type> ys(m, value_type(0, xs[0]));
    for (size_type i = 0; i < m; ++i) ys[i] = fm[m+i][0];
    return ys;
  }

public:
  mod_polynomial() = default;
  mod_polynomial(mod_polynomial const&) = default;
  mod_polynomial(mod_polynomial&&) = default;

  mod_polynomial& operator =(mod_polynomial const&) = default;
  mod_polynomial& operator =(mod_polynomial&&) = default;

  template <typename InputIt>
  mod_polynomial(InputIt first, InputIt last): M_c(first, last) {}
  mod_polynomial(std::initializer_list<value_type> il): M_c(il.begin(), il.end()) {}

  template <typename InputIt>
  void assign(InputIt first, InputIt last) { M_c.assign(first, last); }
  void assign(std::initializer_list<value_type> il) { M_c.assign(il.begin(), il.end()); }

  mod_polynomial& operator +=(mod_polynomial const& other) {
    M_c = S_add(M_c, other.M_c);
    return *this;
  }
  mod_polynomial& operator -=(mod_polynomial const& other) {
    M_c = S_subtract(M_c, other.M_c);
    return *this;
  }

  mod_polynomial operator +(mod_polynomial const& other) const {
    return mod_polynomial(*this) += other;
  }
  mod_polynomial operator -(mod_polynomial const& other) const {
    return mod_polynomial(*this) -= other;
  }

  mod_polynomial& operator *=(mod_polynomial const& other) {
    M_c = S_multiply(M_c, other.M_c);
    return *this;
  }
  mod_polynomial& operator /=(mod_polynomial const& other) {
    M_c = S_quotient(M_c, other.M_c);
    return *this;
  }
  mod_polynomial& operator %=(mod_polynomial const& other) {
    (*this) -= (*this) / other * other;
    return *this;
  }

  mod_polynomial operator *(mod_polynomial const& other) const {
    return mod_polynomial(*this) *= other;
  }
  mod_polynomial operator /(mod_polynomial const& other) const {
    return mod_polynomial(*this) /= other;
  }
  mod_polynomial operator %(mod_polynomial const& other) const {
    return mod_polynomial(*this) %= other;
  }

  // pow_eq
  // pow

  mod_polynomial& operator <<=(size_type k) {
    std::vector<value_type> tmp(k, value_type(0, M_c[0]));
    M_c.insert(M_c.begin(), tmp.begin(), tmp.end());
    return *this;
  }
  mod_polynomial operator <<(size_type k) const {
    return mod_polynomial(*this) <<= k;
  }

  mod_polynomial& operator >>=(size_type k) {
    if (M_c.size() <= k) {
      M_c.erase(M_c.begin()+1, M_c.end());
      M_c[0] = 0;  // for keeping (runtime) mod
    } else {
      M_c.erase(M_c.begin(), M_c.begin()+k);
    }
    return *this;
  }
  mod_polynomial operator >>(size_type k) const {
    return mod_polynomial(*this) >>= k;
  }

  mod_polynomial& operator +=(value_type x) { M_c[0] += x; return *this; }
  mod_polynomial& operator -=(value_type x) { M_c[0] -= x; return *this; }
  mod_polynomial& operator +=(underlying_type x) { M_c[0] += x; return *this; }
  mod_polynomial& operator -=(underlying_type x) { M_c[0] -= x; return *this; }

  mod_polynomial& operator *=(value_type x) {
    if (x == 0) {
      M_c.erase(M_c.begin()+1, M_c.end());
      M_c[0] = 0;
    } else {
      for (auto& c: M_c) c *= x;
    }
    return *this;
  }
  mod_polynomial& operator /=(value_type x) {
    return (*this) *= (1 / x);
  }
  mod_polynomial& operator *=(underlying_type x) {
    if (x == 0) {
      M_c.erase(M_c.begin()+1, M_c.end());
      M_c[0] = 0;
    } else {
      for (auto& c: M_c) c *= x;
    }
    return *this;
  }
  mod_polynomial& operator /=(underlying_type x) {
    return (*this) /= value_type(x, M_c[0]);
  }

  mod_polynomial operator +(value_type x) const {
    return mod_polynomial(*this) += x;
  }
  mod_polynomial operator -(value_type x) const {
    return mod_polynomial(*this) -= x;
  }
  mod_polynomial operator +(underlying_type x) const {
    return mod_polynomial(*this) += x;
  }
  mod_polynomial operator -(underlying_type x) const {
    return mod_polynomial(*this) -= x;
  }

  mod_polynomial operator *(value_type x) const {
    return mod_polynomial(*this) *= x;
  }
  mod_polynomial operator /(value_type x) const {
    return mod_polynomial(*this) /= x;
  }
  mod_polynomial operator *(underlying_type x) const {
    return mod_polynomial(*this) *= x;
  }
  mod_polynomial operator /(underlying_type x) const {
    return mod_polynomial(*this) /= x;
  }

  size_type size() const { return M_c.size(); }

  // coefficient
  value_type operator [](size_type i) const {
    if (i >= M_c.size()) return value_type(0, M_c[0]);
    return M_c[i];
  }

  value_type operator ()(value_type x) const {
    value_type y(0, x);
    value_type xp(1, x);
    for (auto c: M_c) {
      y += c * xp;
      xp *= x;
    }
    return y;
  }

  std::vector<value_type> operator ()(std::vector<value_type> const& xs) const
  { return S_multipoint_evaluation(xs); }

  void inspect() const {
    fprintf(stderr, "f(x): ");
    for (size_type i = 0; i < size(); ++i)
      fprintf(stderr, "%jd%c", M_c[i].get(), i+1<size()? ' ': '\n');
  }
};

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  std::vector<intmax_t> c(n), p(m);
  for (auto& ci: c) scanf("%jd", &ci);
  for (auto& pi: p) scanf("%jd", &pi);
  p.resize(100000);

  mod_polynomial<mi> f(c.begin(), c.end());
  auto y = f(std::vector<mi>(p.begin(), p.end()));
  for (size_t i = 0; i < m; ++i) printf("%jd%c", y[i].get(), i+1<m? ' ': '\n');
}
