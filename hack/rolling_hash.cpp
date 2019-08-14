#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <utility>
#include <tuple>
#include <map>
#include <random>

constexpr intmax_t  operator ""_jd(unsigned long long n) { return n; }
constexpr uintmax_t operator ""_ju(unsigned long long n) { return n; }
constexpr size_t    operator ""_zu(unsigned long long n) { return n; }
// constexpr ptrdiff_t operator ""_td(unsigned long long n) { return n; }

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

template <typename Tp>
Tp modinv(Tp a, Tp mod) {
  Tp x, y;
  gcd(a, mod, x, y);
  x %= mod;
  if (x < 0) x += mod;
  return x;
}

template <typename Tp>
Tp modadd(Tp a, Tp b, Tp mod) {
  a += b % mod;
  if (a < 0) a += mod;
  if (a >= mod) a -= mod;
  return a;
}

template <typename Tp>
Tp modadd(std::initializer_list<Tp> const& adds, Tp mod) {
  Tp res = 0;
  for (auto const& add: adds) {
    res += add % mod;
    if (res < 0) res += mod;
    if (res >= mod) res -= mod;
  }
  return res;
}

template <typename Tp>
Tp modsub(Tp a, Tp b, Tp mod) {
  a -= b % mod;
  if (a < 0) a += mod;
  if (a >= mod) a -= mod;
  return a;
}

template <typename Tp>
Tp modmul(std::initializer_list<Tp> const& muls, Tp mod) {
  Tp res = 1;
  for (auto const& mul: muls) (res *= mul) %= mod;
  return res;
}

template <typename Tp>
Tp modpow(Tp base, intmax_t iexp, Tp mod) {
  Tp res = 1;
  for (Tp dbl = base; iexp; iexp >>= 1) {
    if (iexp & 1) res = res * dbl % mod;
    dbl = dbl * dbl % mod;
  }
  return res;
}

void reduce_basis(std::vector<std::vector<intmax_t>>& f) {
  using value_type = long double;
  auto dot = [](auto const& x, auto const& y) -> value_type {
    value_type res{};
    for (size_t i = 0; i < x.size(); ++i) res += x[i] * y[i];
    return res;
  };
  auto norm = [&](auto const& x) -> value_type { return dot(x, x); };
  auto gso = [&](auto const& g, auto& r, auto& u) -> void {
    size_t n = g.size();
    for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < i; ++j)
        u[i][j] = dot(g[i], r[j]) / norm(r[j]);
      r[i] = g[i];
      for (size_t j = 0; j < i; ++j) {
        for (size_t k = 0; k < n; ++k)
          r[i][k] -= u[i][j] * r[j][k];
      }
    }
  };

  size_t n = f.size();
  std::vector<std::vector<value_type>> g(n), r(n), u(n);
  for (size_t i = 0; i < n; ++i) {
    g[i].resize(n);
    for (size_t j = 0; j < n; ++j) g[i][j] = f[i][j];
    r[i].resize(n);
    u[i].resize(i);
  }
  gso(g, r, u);
  constexpr value_type half = 0.5;
  for (size_t i = 1; i < n;) {
    for (size_t j = i; j--;) {
      if (std::abs(u[i][j]) < half) continue;
      value_type c = std::floor(u[i][j] + half);
      for (size_t k = 0; k < n; ++k) g[i][k] -= c * g[j][k];
      uintmax_t cz = c;
      for (size_t k = 0; k < n; ++k) f[i][k] -= cz * f[j][k];
      gso(g, r, u);
    }
    if (i > 0 && norm(r[i-1]) > 2 * norm(r[i])) {
      std::swap(g[i-1], g[i]);
      std::swap(f[i-1], f[i]);
      gso(g, r, u);
      --i;
    } else {
      ++i;
    }
  }
}

std::pair<std::vector<intmax_t>, std::vector<intmax_t>> make_collision(
    intmax_t P, intmax_t B, int sigma
) {
  size_t n = 4;
  std::vector<int> poly;
  do {
    std::vector<std::vector<intmax_t>> L(n, std::vector<intmax_t>(n));
    for (size_t i = 0; i+1 < n; ++i) {
      L[i][i] = -B;
      L[i][i+1] = 1;
    }
    L[n-1][0] = P;
    reduce_basis(L);

    bool done = true;
    for (size_t i = 0; i < n; ++i) {
      done &= (std::abs(L[0][i]) < sigma);
    }
    for (size_t i = 0; i < n; ++i) fprintf(stderr, "%jd%c", L[0][i], i+1<n? ' ': '\n');
    if (!done) {
      n += 4;
      continue;
    }
    poly.resize(n);
    for (size_t i = 0; i < n; ++i) poly[i] = L[0][i];
    break;
  } while (true);

  while (n > 1 && poly[n-1] == 0) --n;

  std::vector<intmax_t> s(n), t(n);
  for (size_t i = 0; i < n; ++i) {
    int c = poly[n-1-i];
    s[i] = (c >= 0)? c: 0;
    t[i] = (c >= 0)? 0: -c;
  }
  return {s, t};
}

void replace(std::vector<intmax_t>& s,
             std::vector<std::vector<intmax_t>> const& t) {

  std::vector<intmax_t> res;
  for (auto i: s) {
    res.insert(res.end(), t.at(i).begin(), t.at(i).end());
  }
  s = std::move(res);
}

intmax_t hash(std::string const& s, intmax_t p, intmax_t b) {
  intmax_t h = 0;
  for (auto c: s)
    h = (static_cast<__int128>(h)*b + c) % p;
  fprintf(stderr, "h: %jd\n", h);
  return h;
}

intmax_t hash(std::vector<intmax_t> const& s, intmax_t p, intmax_t b) {
  intmax_t h = 0;
  for (auto c: s)
    h = (static_cast<__int128>(h)*b + c) % p;
  return h;
}

std::pair<std::vector<intmax_t>, std::vector<intmax_t>> make_collision(
    std::vector<std::pair<intmax_t, intmax_t>> const& params, size_t sigma
) {
  std::vector<intmax_t> mods, bases;
  for (auto& pi: params) {
    intmax_t mod, base;
    std::tie(mod, base) = pi;
    mods.push_back(mod);
    bases.push_back(base);
  }

  std::mt19937 rsk(0315);
  std::uniform_int_distribution<int> nya(0, 1);

  std::vector<intmax_t> s, t;
  std::tie(s, t) = make_collision(mods[0], bases[0], sigma);
  for (size_t i = 1; i < mods.size(); ++i) {
    intmax_t hs = hash(s, mods[i], bases[i]);
    intmax_t ht = hash(t, mods[i], bases[i]);
    if (hs == ht) continue;

    size_t len = std::log(std::sqrt(mods[i])) / std::log(2);
    std::vector<intmax_t> s0(len), t0;
    intmax_t r = modpow<__int128>(bases[i], s.size(), mods[i]);
    do {
      std::map<intmax_t, std::vector<intmax_t>> seen;
      size_t fail = 0;
      do {
        // birthday attack
        for (auto& si: s0) si = nya(rsk)? hs: ht;
        intmax_t h0 = hash(s0, mods[i], r);
        auto it = seen.find(h0);
        if (it != seen.end()) {
          if (s0 == it->second) {
            if (++fail == 20) {
              s0.push_back(0);
              break;
            }
            continue;
          }
          t0 = it->second;
          break;
        }
        seen.emplace(h0, s0);
      } while (true);
    } while (t0.empty());
    std::vector<std::vector<intmax_t>> u{s, t};
    for (auto& si: s0) si = ((si == hs)? 1: 0);
    for (auto& ti: t0) ti = ((ti == hs)? 1: 0);
    replace(s0, u);
    replace(t0, u);
    s = std::move(s0);
    t = std::move(t0);
  }

  return {s, t};
}

void print_strings(std::string const& s, std::string const& t, bool rich) {
  if (!rich) {
    puts(s.c_str()), puts(t.c_str());
    return;
  }

  for (size_t i = 0; i < s.length(); ++i) {
    if (s[i] != t[i]) printf("\x1b[1;31m");
    printf("%c", s[i]);
    if (s[i] != t[i]) printf("\x1b[m");
  }
  puts("");

  for (size_t i = 0; i < s.length(); ++i) {
    if (s[i] != t[i]) printf("\x1b[1;31m");
    printf("%c", t[i]);
    if (s[i] != t[i]) printf("\x1b[m");
  }
  puts("");
}

int main() {
  size_t n;
  scanf("%zu", &n);

  std::vector<std::pair<intmax_t, intmax_t>> params;
  // params.emplace_back(1000000007, 10007);
  // params.emplace_back(4294967296_jd, 1000000007);
  // params.emplace_back(1000000007, 1777771);
  for (size_t i = 0; i < n; ++i) {
    intmax_t mod, base;
    scanf("%jd %jd", &mod, &base);
    params.emplace_back(mod, base);
  }

  std::sort(params.rbegin(), params.rend());

  std::vector<intmax_t> s0, t0;
  std::tie(s0, t0) = make_collision(params, 26);

  std::string s, t;
  for (auto c: s0) s += c+'a';
  for (auto c: t0) t += c+'a';

  fprintf(stderr, "length: %zu\n", s.length());
  print_strings(s, t, true);

  for (auto const& pi: params) {
    intmax_t mod, base;
    std::tie(mod, base) = pi;
    assert(hash(s, mod, base) == hash(t, mod, base));
  }
}
