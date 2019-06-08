#include <cstdio>
#include <cstdint>
#include <vector>
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
      if (M_c[i|m] < value) {
        i |= m;
        value -= M_c[i];
      }
      m >>= 1;
    }
    return {i, value};
  }

  void inspect() const {
    for (size_t i = 1; i < M_c.size(); ++i)
      fprintf(stderr, "%d%c", M_c[i], i+1<M_c.size()? ' ':'\n');
  }
};

class bit_vector {
  using value_type = uintmax_t;
  static size_t constexpr S_word = 64;

  std::deque<value_type> M_c;
  std::deque<size_t> M_bits;
  prefix_sum<size_t> M_bits_sum, M_ones_sum;

  void M_insert(size_t i0, size_t i1, int b) {
    if (M_bits[i0] == S_word) {
      M_break(i0);
      if (i1 > S_word/2) {
        ++i0;
        i1 -= S_word/2;
      }
    }
    // ...

    ++M_bits[i0];
    M_bits_sum.add(i0, 1);
    if (b) M_ones_sum.add(i0, 1);
  }

  void M_erase(size_t i0, size_t i1) {
    --M_bits[i0];
    if (M_c[i0] >> i1 & 1) M_ones_sum.add(i0, -1);
    M_bits_sum.add(i0, -1);
    // ...

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
  bool operator [](size_t i) const;

  size_t rank(size_t i, bool b) const;
  size_t select(size_t i, bool b) const;
};

int main() {
  uintmax_t 
}
