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
  using value_type = std::array<uintmax_t, 4>;
  static size_t constexpr S_word = 64 * 4;

  size_t M_size = 0;
  std::deque<value_type> M_c{{}};
  std::deque<size_t> M_bits{0};
  prefix_sum<size_t> M_bits_sum{0}, M_ones_sum{0};

  // static value_type S_mask(size_t k) { return (value_type(1) << k) - 1; }
  static int S_popcount(value_type const& x) {
    int res = 0;
    for (auto xi: x) res += __builtin_popcountll(xi);
    // res += __builtin_popcountll(x & S_mask(64));
    // res += __builtin_popcountll(x >> 64);
    return res;
  }
  static void S_mini_insert(value_type& x, size_t i, int b) {
    size_t j0 = 3 - i / 64;
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

  static value_type S_mini_erase(value_type& x, size_t i);
  static uintmax_t S_mini_access(value_type const& x, size_t i) {
    size_t j0 = 3 - i / 64;
    size_t j1 = i % 64;
    return x[j0] >> j1 & 1;
  }
  static uintmax_t S_mini_mask(size_t i) { return (uintmax_t(1) << i) - 1; }
  static void S_lower(value_type& x) {
    x[0] = x[1] = 0;
  }
  static void S_upper(value_type& x) {
    x[2] = x[0];
    x[3] = x[1];
    x[0] = x[1] = 0;
  }

  void M_break(size_t i) {
    assert(M_bits[i] == S_word);
    M_bits.insert(M_bits.begin()+i, M_bits[i]);
    M_bits[i] = M_bits[i+1] = S_word/2;
    M_c.insert(M_c.begin()+i, M_c[i]);
    // M_c[i] &= S_mask(S_word/2);
    // M_c[i+1] >>= S_word/2;
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
    // {
    //   value_type prev = M_c[i0];
    //   assert(i1 < S_word);
    //   value_type hi = ((prev >> i1) << 1 | b) << i1;
    //   value_type lo = prev & S_mask(i1);
    //   M_c[i0] = hi | lo;
    // }

    ++M_bits[i0];
    M_bits_sum.add(i0, 1);
    if (b) M_ones_sum.add(i0, 1);
  }

  void M_erase(size_t i0, size_t i1) {
    --M_size;
    --M_bits[i0];
    // if (M_c[i0] >> i1 & 1) M_ones_sum.add(i0, -1);
    if (S_mini_access(M_c[i0], i1)) M_ones_sum.add(i0, -1);
    M_bits_sum.add(i0, -1);
    S_mini_erase(M_c[i0], i1);
    // {
    //   value_type prev = M_c[i0];
    //   assert(i1 < S_word);
    //   value_type hi = (prev >> i1 >> 1) << i1;
    //   value_type lo = prev & S_mask(i1);
    //   M_c[i0] = hi | lo;
    // }

    if (M_bits[i0] < S_word/4) {
      // try to merge?
    }
  }

public:
  bit_vector() = default;
  bit_vector(bit_vector const&) = default;
  bit_vector(bit_vector&&) = default;

  void insert(size_t i, bool b) {
    size_t j0, j1;
    std::tie(j0, j1) = M_bits_sum.upto(i);
    // fprintf(stderr, "j0/j1: %zu/%zu\n", j0, j1);
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
    // fprintf(stderr, "[%zu]: %zu/%zu\n", i, j0, j1);
    // return M_c[j0] >> j1 & 1;
    return S_mini_access(M_c[j0], j1);
  }

  size_t rank(size_t i, bool b) const;
  size_t select(size_t i, bool b) const;

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
  size_t n = 100000;
  std::vector<int> naive;
  for (size_t i = 0; i < n; ++i) {
    // if (i % 10000 == 0) fprintf(stderr, "%zu\n", i);
    std::uniform_int_distribution<size_t> rng(0, i);
    size_t pos = rng(rsk);
    bool bit = rbg(rsk);
    // fprintf(stderr, "(%zu) insert %d into %zu\n", i, bit, pos);
    bv.insert(pos, bit);
    bv.inspect(pos);
    naive.insert(naive.begin()+pos, bit);
    for (size_t j = 0; j <= i; ++j)
      assert(bv[j] == naive[j]);
  }
  // bv.inspect();
  return 0;
}

int main() {
  random_test();
}
