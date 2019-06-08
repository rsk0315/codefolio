#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <deque>
#include <algorithm>
#include <utility>
#include <memory>
#include <functional>

template <typename Tp, typename Adder = std::plus<Tp>>
class prefix_sum {
public:
  using value_type = Tp;
  using adder = Adder;

private:
  std::vector<value_type> M_c;

  void M_build() {
    size_t n = M_c.size()-1;
    for (size_t i = 1; i <= n; ++i) {
      size_t j = i + (i & -i);
      if (j <= n) M_c[j] += M_c[i];
    }
  }

public:
  prefix_sum(): M_c(1) {}
  prefix_sum(prefix_sum const&) = default;
  prefix_sum(prefix_sum&&) = default;
  prefix_sum(size_t n, value_type const& x = value_type{}): M_c(n+1, x) {
    M_c[0] = value_type{};
    M_build();
  }
  template <typename InputIt>
  prefix_sum(InputIt first, InputIt last): M_c(first, last) {
    M_c.insert(M_c.begin(), value_type{});
    M_build();
  }
  prefix_sum(std::initializer_list<value_type> ilist): prefix_sum(ilist.begin(), ilist.end()) {}

  prefix_sum& operator =(prefix_sum const&) = default;
  prefix_sum& operator =(prefix_sum&&) = default;

  void add(size_t i, value_type const& x) {
    // 1-indexing, internally
    ++i;
    size_t n = M_c.size()-1;
    while (i <= n) {
      M_c[i] += x;
      i += (i & -i);
    }
  }

  value_type accumulate(size_t i) const {
    // exclusive in 0-indexing, equivalent to inclusive in 1-indexing
    value_type res{};
    while (i > 0) {
      res += M_c[i];
      i ^= (i & -i);
    }
    return res;
  }

  void push_back(value_type const& x) {
    size_t n = M_c.size();
    M_c.push_back(x);
    size_t i = 1;
    size_t j = n-1;
    while (j & i) {
      M_c[n] += M_c[j];
      j ^= i;
      i <<= 1;
    }
  }

  std::pair<size_t, value_type> upto(value_type value) const {
    // assuming 0 <= a[i] for 1 <= i <= n
    // returns {i, x} that satisfy
    // (1) maximum i such that accumulate(i) < value
    // (2) accumulate(i, x) = value
    size_t n = M_c.size()-1;
    if (n == 0) return {0, 0};  // ???
    {
      value_type tmp = accumulate(n);
      if (tmp < value) return {n, value-tmp};
    }
    size_t m = size_t(1) << (63 - __builtin_clzll(n));
    size_t i = 0;
    while (m > 0) {
      if ((i|m) < M_c.size() && M_c[i|m] < value) {
        i |= m;
        value -= M_c[i];
      }
      m >>= 1;
    }
    return {i, value};
  }

  void inspect() const {
    for (size_t i = 1; i < M_c.size(); ++i)
      fprintf(stderr, "%zu%c", M_c[i], i+1<M_c.size()? ' ':'\n');
  }
};

class bit_vector {
  static size_t constexpr S_unit = 64;
  using value_type = std::array<uintmax_t, S_unit>;
  static size_t constexpr S_word = 64 * S_unit;

  size_t M_size = 0;
  std::deque<value_type> M_c{{}};
  std::deque<size_t> M_bits{0};
  prefix_sum<size_t> M_bits_sum{0}, M_ones_sum{0};

  static int S_popcount(value_type const& x) {
    int res = 0;
    for (auto xi: x) res += __builtin_popcountll(xi);
    return res;
  }
  static void S_mini_insert(value_type& x, size_t i, int b) {
    size_t j0 = S_unit - 1 - i / 64;
    size_t j1 = i % 64;
    int cy = x[j0] >> 63 & 1;
    {
      uintmax_t hi = ((x[j0] >> j1) << 1 | b) << j1;
      uintmax_t lo = x[j0] & S_mini_mask(j1);
      x[j0] = hi | lo;
    }
    while (j0--) {
      int tmp = x[j0] >> 63 & 1;
      x[j0] = x[j0] << 1 | cy;
      cy = tmp;
    }
  }

  static void S_mini_erase(value_type& x, size_t i) {
    size_t j0 = S_unit - 1 - i / 64;
    size_t j1 = i % 64;
    {
      uintmax_t hi = (x[j0] >> j1 >> 1) << j1;
      uintmax_t lo = x[j0] & S_mini_mask(j1);
      x[j0] = hi | lo;
    }
    while (j0--) {
      x[j0+1] |= (x[j0] & 1) << 63;
      x[j0] >>= 1;
    }
  }
  static uintmax_t S_mini_access(value_type const& x, size_t i) {
    size_t j0 = S_unit - 1 - i / 64;
    size_t j1 = i % 64;
    return x[j0] >> j1 & 1;
  }
  static uintmax_t S_mini_mask(size_t i) { return (uintmax_t(1) << i) - 1; }
  static void S_lower(value_type& x) {
    for (size_t i = 0; i < S_unit/2; ++i)
      x[i] = 0;
  }
  static void S_upper(value_type& x) {
    for (size_t i = 0; i < S_unit/2; ++i) {
      x[i+S_unit/2] = x[i];
      x[i] = 0;
    }
  }

  static size_t S_rank(value_type const& x, size_t i) {
    size_t j0 = S_unit - 1 - i / 64;
    size_t j1 = i % 64;
    size_t res = __builtin_popcountll(x[j0] & S_mini_mask(j1));
    for (size_t j = j0+1; j < S_unit; ++j)
      res += __builtin_popcountll(x[j]);
    return res;
  }
  static size_t S_select(value_type const& x, size_t i);

  void M_break(size_t i) {
    assert(M_bits[i] == S_word);
    M_bits.insert(M_bits.begin()+i, M_bits[i]);
    M_bits[i] = M_bits[i+1] = S_word/2;
    M_c.insert(M_c.begin()+i, M_c[i]);
    S_lower(M_c[i]);
    S_upper(M_c[i+1]);

    M_bits_sum = prefix_sum<size_t>(M_bits.begin(), M_bits.end());
    std::vector<size_t> ones(M_c.size());
    for (size_t i = 0; i < M_c.size(); ++i)
      ones[i] = S_popcount(M_c[i]);
    M_ones_sum = prefix_sum<size_t>(ones.begin(), ones.end());
  }

  void M_insert(size_t i0, size_t i1, int b) {
    ++M_size;
    if (M_bits[i0] == S_word) {
      M_break(i0);
      if (i1 > S_word/2) {
        ++i0;
        i1 -= S_word/2;
      }
    }
    S_mini_insert(M_c[i0], i1, b);

    ++M_bits[i0];
    M_bits_sum.add(i0, 1);
    if (b) M_ones_sum.add(i0, 1);
  }

  void M_erase(size_t i0, size_t i1) {
    --M_size;
    --M_bits[i0];
    if (S_mini_access(M_c[i0], i1)) M_ones_sum.add(i0, -1);
    M_bits_sum.add(i0, -1);
    S_mini_erase(M_c[i0], i1);

    if (M_bits[i0] < S_word/4) {
      // try to merge?
    }
  }

  size_t M_select1(size_t i) const {
    size_t j0, j1;
    std::tie(j0, j1) = M_ones_sum.upto(i);
    size_t res = S_word * j0;
    res += S_select(M_c[j0], j1);
    return res;
  }
  size_t M_select0(size_t i) const {
    // ???
    return -1;
  }

public:
  bit_vector() = default;
  bit_vector(bit_vector const&) = default;
  bit_vector(bit_vector&&) = default;

  void insert(size_t i, bool b) {
    size_t j0, j1;
    std::tie(j0, j1) = M_bits_sum.upto(i);
    M_insert(j0, j1, b);
  }
  void erase(size_t i) {
    size_t j0, j1;
    std::tie(j0, j1) = M_bits_sum.upto(i);
    if (M_bits[j0] == j1) {
      ++j0;
      j1 = 0;
    }
    M_erase(j0, j1);
  }
  bool operator [](size_t i) const {
    size_t j0, j1;
    std::tie(j0, j1) = M_bits_sum.upto(i);
    if (M_bits[j0] == j1) {
      ++j0;
      j1 = 0;
    }
    return S_mini_access(M_c[j0], j1);
  }

  size_t rank(int b, size_t i) const {
    size_t j0, j1;
    std::tie(j0, j1) = M_bits_sum.upto(i);
    size_t res = M_ones_sum.accumulate(j0);
    res += S_rank(M_c[j0], j1);
    if (!b) res = i - res;
    return res;
  }

  size_t rank0(size_t i) const { return rank(0, i); }
  size_t rank1(size_t i) const { return rank(1, i); }

  size_t select(size_t i, bool b) const { return b? M_select1(i) : M_select0(i); }

  void inspect(size_t pos = -1) const {
    M_bits_sum.inspect();
    int const word = 32;
    for (size_t i = 0; i < M_bits.size(); ++i)
      fprintf(stderr, "%zu%c", M_bits[i], i+1<M_bits.size()? ' ':'\n');
    for (size_t i = 0; i < M_size; ++i) {
      if (i == pos) fprintf(stderr, "\x1b[1;31m");
      fprintf(stderr, "%d", (*this)[i]);
      if (i == pos) fprintf(stderr, "\x1b[m");
      fprintf(stderr, "%c", (i+1 < M_size && i % (word/2) != (word/2-1))? ' ':'\n');
    }
  }
};

#include <random>

int random_test() {
  bit_vector bv;
  std::mt19937 rsk(0315);
  std::uniform_int_distribution<int> rbg(0, 1);
  size_t n = 1000000;
  // std::vector<int> naive;
  size_t m = 0;
  for (size_t i = 0; i < n; ++i) {
    if (m == 0 || rbg(rsk)) {
      // insert
      std::uniform_int_distribution<size_t> rng(0, m);
      size_t pos = rng(rsk);
      bool bit = rbg(rsk);
      // fprintf(stderr, "(%zu) insert %d into %zu\n", i, bit, pos);
      bv.insert(pos, bit);
      // naive.insert(naive.begin()+pos, bit);
      ++m;
    } else {
      std::uniform_int_distribution<size_t> rng(0, m-1);
      size_t pos = rng(rsk);
      bv.erase(pos);
      // naive.erase(naive.begin()+pos);
      --m;
    }
    // bv.inspect();
    // for (size_t j = 0; j < m; ++j)
    //   assert(bv[j] == naive[j]);
  }
  // bv.inspect();
  return 0;
}

template <typename Tp, size_t bitlen = 8 * sizeof(Tp)>
class wavelet_matrix {
public:
  using value_type = Tp;

private:
  std::vector<size_t> zeros;
  size_t n;
  std::array<bit_vector, bitlen> a;

  size_t start_index(value_type x) const {
    size_t s = 0;
    size_t t = 0;
    for (size_t i = bitlen; i-- > 1;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return s;
  }

public:
  wavelet_matrix(): zeros(bitlen), n(0) {}

  template <class ForwardIt>
  wavelet_matrix(ForwardIt first, ForwardIt last): zeros(bitlen) {
    std::vector<value_type> c(first, last);
    n = c.size();
    std::vector<value_type> whole = c;
    for (size_t i = bitlen; i--;) {
      std::vector<value_type> zero, one;
      std::vector<bool> vb(n);
      for (size_t j = 0; j < n; ++j) {
        ((whole[j] >> i & 1)? one:zero).push_back(whole[j]);
        vb[j] = (whole[j] >> i & 1);
      }

      zeros[bitlen-i-1] = zero.size();
      a[bitlen-i-1] = bit_vector(vb);
      if (i == 0) break;
      whole = std::move(zero);
      whole.insert(whole.end(), one.begin(), one.end());
    }
  }

  size_t size() const { return a[0].size(); }
  bool empty() const { return a[0].empty(); }

  size_t rank(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return t - s;
  }

  std::pair<bool, value_type> max_lt(value_type x, size_t s, size_t t) const {
    return max_le(x-1, s, t);
  }
  std::pair<bool, value_type> max_le(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 1);
      if (reverted) tg = 0;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 0;

      reverted = false;
      if (tg == 1) {
        if (ok0) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok1) {
          ch = 1;
        } else {
          tight = false;
        }
      } else if (!ok0 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << i) - 1;
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= value_type(1) << i;
      }
    }
    return {true, res};
  }
  std::pair<bool, value_type> min_gt(value_type x, size_t s, size_t t) const {
    return min_ge(x+1, s, t);
  }
  std::pair<bool, value_type> min_ge(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 0);
      if (reverted) tg = 1;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 1;

      reverted = false;
      if (tg == 0) {
        if (ok1) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok0) {
          ch = 0;
        } else {
          tight = false;
        }
      } else if (!ok1 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << ri) - 1;  // suspicious
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= static_cast<value_type>(1) << i;
      }
    }
    return {true, res};
  }

  value_type quantile(size_t k, size_t s, size_t t) const {
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      if (k < z) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        res |= static_cast<value_type>(1) << i;
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        k -= z;
      }
    }
    return res;
  }

  std::array<size_t, 3> rank_three_way(value_type x, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        size_t d = tmp - (t-s);
        eq -= d;
        lt += d;
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
        size_t d = tmp - (t-s);
        eq -= d;
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  std::array<size_t, 3> xored_rank_three_way(value_type x, value_type y, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if ((x ^ y) >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }

      size_t d = tmp - (t-s);
      eq -= d;
      if (y >> i & 1) {
        lt += d;
      } else {
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  size_t select(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t si = start_index(x);
    t += a[bitlen-1].rank(x & 1, si);
    t = a[bitlen-1].select(x & 1, t);

    for (size_t i = 1; i < bitlen; ++i) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) t -= zeros[j];
      t = a[j].select(x >> i & 1, t);
    }

    return t;
  }

  value_type operator [](size_t t) const {
    value_type res = 0;
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      if (a[j][s+t]) {
        t = a[j].rank(1, s+t+1) - 1;
        s = zeros[j];
        res |= static_cast<value_type>(1) << i;
      } else {
        t = a[j].rank(0, s+t+1) - 1;
        s = 0;
      }
    }
    return res;
  }

  void insert(size_t t, value_type x) {
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      a[j].insert(s+t, x >> i & 1);
      if (x >> i & 1) {
        t = a[j].rank(1, s+t+1) - 1;
        s = zeros[j];
      } else {
        t = a[j].rank(0, s+t+1) - 1;
        s = 0;
        ++zeros[j];
      }
    }
    ++n;
  }

  void erase(size_t t) {
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t u = s+t;
      int x = a[j][u];
      if (x) {
        t = a[j].rank(1, u+1) - 1;
        s = zeros[j];
      } else {
        t = a[j].rank(0, u+1) - 1;
        s = 0;
        --zeros[j];
      }
      a[j].erase(u);
    }
    --n;
  }

  void update(size_t t, value_type x) {
    erase(t);
    insert(t, x);
  }

  void inspect() const {
    for (size_t i = 0; i < bitlen; ++i) {
      fprintf(stderr, "%zu (%zu): ", i, zeros[i]);
      for (size_t j = 0; j < n; ++j)
        fprintf(stderr, "%d%c", a[i][j], j+1<n? ' ':'\n');
    }
  }
};

int dp_q() {
  size_t n;
  scanf("%zu", &n);

  std::vector<intmax_t> h(n), a(n);
  for (auto& hi: h) scanf("%jd", &hi);
  for (auto& ai: a) scanf("%jd", &ai);

  wavelet_matrix<intmax_t, 48> wm;
  for (size_t i = 0; i < n; ++i) wm.insert(i, 0);

  for (size_t i = 0; i < n; ++i) {
    size_t j = h[i];
    auto qt = wm.quantile(j-1, 0, j);
    wm.update(j-1, qt + a[i]);
  }

  printf("%jd\n", wm.quantile(n-1, 0, n));
  return 0;
}

int hack() {
  bit_vector bv;
  for (int i = 0; i < 64*64; ++i) bv.insert(0, 1);
  for (int i = 0; i < 64*64; ++i) bv.insert(0, 1);
  bv.insert(0, 1);
  for (int i = 0; i < 64*64; ++i) bv.erase(0);
  for (int i = 0; i < 64*64; ++i) bv.erase(0);
  // kore ha dame
  // 0 18446744073709547521 2048 1
  bv.inspect();
  return 1;
}

int main() {
  dp_q();
}
