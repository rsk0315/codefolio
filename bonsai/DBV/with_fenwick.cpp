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

// class bit_vector {
//   using value_type = uintmax_t;

//   std::vector<value_type> M_c;
//   prefix_sum<size_t> M_offset;

// public:
//   bit_vector() = default;
//   bit_vector(bit_vector const&) = default;
//   bit_vector(bit_vector&&) = default;

//   void insert(size_t i, bool b) {
//     size_t j = M_offset.lower_bound(i);
//     size_t k = i - M_offset.accumulate(j-1);
//     // ...
//     // if size[j] == 64, break it into two pieces
//     M_offset.add(i, 1);
//   }
//   void erase(size_t i) {
//     size_t j = M_offset.lower_bound(i);
//     size_t k = i - M_offset.accumulate(j-1);
//     // ...
//     // if size[j] == 16, try to merge with adjacent one
//     M_offset.add(i, -1);
//   }
//   bool operator [](size_t i) const;

//   size_t rank(size_t i, bool b) const;
//   size_t select(size_t i, bool b) const;
// }

int main() {
  // size_t n = 30;
  // std::vector<int> base(n);
  // for (size_t i = 0; i < n; ++i) base[i] = 1 << i;
  // for (size_t i = 0; i < n; ++i)
  //   fprintf(stderr, "%d%c", base[i], i+1<n? ' ':'\n');
  // // prefix_sum<int> ps(base.begin(), base.end()-2);
  // // ps.push_back(1 << 10);
  // // // ++n;
  // // ps.push_back(1 << 11);
  // // // ++n;
  // // ps.inspect();
  // prefix_sum<int> ps;
  // for (size_t i = 0; i <= n; ++i) ps.push_back(1 << i);
  // for (size_t i = 1; i <= n; ++i)
  //   fprintf(stderr, "%d%c", ps.accumulate(i), i<n? ' ':'\n');

  prefix_sum<int> ps{1, 4, 6, 2, 9, 3};
  for (int i = 0; i <= 27; ++i) {
    size_t j;
    int k;
    std::tie(j, k) = ps.upto(i);
    printf("%d: %zu/%d\n", i, j, k);
  }
}
