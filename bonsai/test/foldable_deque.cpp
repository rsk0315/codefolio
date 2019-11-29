#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <deque>
#include <stack>
#include <set>
#include <tuple>

#include "../../DataStructure/foldable_deque.cpp"

template <intmax_t Base = 943681729, intmax_t Modulo = 1000000007>
class monoid_rolling_hash {
public:
  using value_type = intmax_t;

private:
  value_type M_hash = 0;
  value_type M_basepow = 1;

public:
  monoid_rolling_hash() = default;
  monoid_rolling_hash(monoid_rolling_hash const&) = default;
  monoid_rolling_hash(monoid_rolling_hash&&) = default;

  monoid_rolling_hash& operator =(monoid_rolling_hash const&) = default;
  monoid_rolling_hash& operator =(monoid_rolling_hash&&) = default;

  template <typename Tp>
  monoid_rolling_hash(Tp x) {
    if ((x %= Modulo) < 0) x += Modulo;
    M_hash = x;
    M_basepow = Base % Modulo;
  }

  template <typename InputIt>
  monoid_rolling_hash(InputIt first, InputIt last) {
    while (first != last) {
      (M_hash = M_hash * Base + *first++) %= Modulo;
      (M_basepow *= Base) %= Modulo;
    }
    if (M_hash < 0) M_hash += Modulo;
  }

  monoid_rolling_hash& fold_eq(monoid_rolling_hash const& other) {
    // (h1, b1) + (h2, b2) = (h1*b2 + h2, b1*b2)
    (M_hash = M_hash * other.M_basepow + other.M_hash) %= Modulo;
    (M_basepow *= other.M_basepow) %= Modulo;
    return *this;
  }

  monoid_rolling_hash fold(monoid_rolling_hash const& other) const {
    return monoid_rolling_hash(*this).fold_eq(other);
  }

  value_type get() const { return M_hash; }
};

class monoid_rolling_hashes {
public:
  using value_type = std::tuple<intmax_t, intmax_t, intmax_t>;

private:
  using rh1 = monoid_rolling_hash<>;
  using rh2 = monoid_rolling_hash<943681729, 998244353>;
  using rh3 = monoid_rolling_hash<943681729, 1000000009>;

  rh1 M_h1;
  rh2 M_h2;
  rh3 M_h3;

public:
  monoid_rolling_hashes() = default;
  monoid_rolling_hashes(monoid_rolling_hashes const&) = default;
  monoid_rolling_hashes(monoid_rolling_hashes&&) = default;

  template <typename Tp>
  monoid_rolling_hashes(Tp x): M_h1(x), M_h2(x), M_h3(x) {}

  monoid_rolling_hashes& operator =(monoid_rolling_hashes const&) = default;
  monoid_rolling_hashes& operator =(monoid_rolling_hashes&&) = default;

  monoid_rolling_hashes& fold_eq(monoid_rolling_hashes const& other) {
    M_h1.fold_eq(other.M_h1);
    M_h2.fold_eq(other.M_h2);
    M_h3.fold_eq(other.M_h3);
    return *this;
  }

  monoid_rolling_hashes fold(monoid_rolling_hashes const& other) const {
    return monoid_rolling_hashes(*this).fold_eq(other);
  }

  value_type get() const { return std::make_tuple(M_h1.get(), M_h2.get(), M_h3.get()); }
};

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  char buf[393216];
  scanf("%s", buf);
  std::string s = buf;

  foldable_deque<monoid_rolling_hashes> fd;
  size_t l = 0;
  size_t r = 1;
  fd.emplace_back(s[0]);

  std::set<std::tuple<intmax_t, intmax_t, intmax_t>> seen;
  for (size_t i = 0; i < m; ++i) {
    char buf[4];
    scanf("%s", buf);
    std::string q = buf;

    if (q == "L++") {
      ++l;
      fd.pop_front();
    } else if (q == "L--") {
      fd.emplace_front(s[--l]);
    } else if (q == "R++") {
      fd.emplace_back(s[r++]);
    } else if (q == "R--") {
      --r;
      fd.pop_back();
    }

    seen.insert(fd.fold().get());
  }

  printf("%zu\n", seen.size());
}
