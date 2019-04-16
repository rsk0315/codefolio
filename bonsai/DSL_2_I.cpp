#define _GLIBCXX_DEBUG
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <limits>

template <class Tp, class BinaryOperation1, class BinaryOperation2>
class segment_tree {
public:
  using binary_operation1 = BinaryOperation1;
  using binary_operation2 = BinaryOperation2;
  using value_type = Tp;

private:
  size_t m, h;
  std::vector<value_type> c, d;
  binary_operation1 op1;  // aggregate
  binary_operation2 op2;  // update

  size_t height(size_t n) const {
    return 64 - __builtin_clzll(n);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

  void calc(size_t p, size_t k) {
    // called in build()
    c[p] = op1(c[p<<1], c[p<<1|1]);
    if (p < m/2) c[p] = op2(c[p], d[p], k);
  }

  void apply(size_t i, Tp x, size_t k) {
    // called in push() and update()
    c[i] = op2(c[i], x, k);
    if (i < m/2) d[i] = op2(d[i], x);
  }

  void build(size_t i) {
    size_t k = 2;
    for (i += m/2; i > 1; k <<= 1) {
      i >>= 1;
      calc(i, k);
    }
  }

  void push(size_t i) {
    i += m/2;
    for (size_t s = h, k = 1 << (h-1); s > 0; --s, k >>= 1) {
      size_t j = i >> s;
      if (d[j] != op2.identity) {
        apply(j<<1|0, d[j], k);
        apply(j<<1|1, d[j], k);
        d[j] = op2.identity;
      }
    }
  }

public:
  segment_tree(size_t n,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*n), h(height(n)), c(m, op1.identity), d(n, op2.identity),
    op1(op1), op2(op2)
  {}

  segment_tree(size_t n, const value_type& e,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*n), h(height(n)), c(m, e), d(n, op2.identity), op1(op1), op2(op2)
  {
    for (size_t i = n; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*std::distance(first, last)), h(height(m/2)), c(m),
    d(m/2, op2.identity), op1(op1), op2(op2)
  {
    static_assert(std::is_same<Tp, typename ForwardIt::value_type>::value, "");
    for (size_t i = m/2; first != last; ++i) c[i] = *first++;
    init_aggregate();
  }

  void update(size_t l, size_t r, Tp x) {
    push(l);
    push(r-1);
    size_t l0 = l;
    size_t r0 = r;
    l += m/2;
    r += m/2;
    size_t k = 1;
    while (l < r) {
      if (l & 1) apply(l++, x, k);
      if (r & 1) apply(--r, x, k);
      l >>= 1;
      r >>= 1;
      k <<= 1;
    }
    build(l0);
    build(r0-1);
  }

  value_type aggregate(size_t l, size_t r) {
    push(l);
    push(r-1);
    value_type resl = op1.identity;
    value_type resr = op1.identity;
    l += m/2;
    r += m/2;
    while (l < r) {
      if (l & 1) resl = op1(resl, c[l++]);
      if (r & 1) resr = op1(c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return op1(resl, resr);
  }
};

template <class Tp>
struct find {
  using pair_type = std::pair<Tp, bool>;
  pair_type identity = {0, false};
  pair_type operator ()(const pair_type& x, const pair_type& y) const {
    return {x.first + y.first, false};
  }
};

template <class Tp>
struct update {
  using pair_type = std::pair<Tp, bool>;
  pair_type identity = {0, false};
  pair_type operator ()(const pair_type& x, const pair_type& y) const {
    return y.second? y:x;
  }
  pair_type operator ()(const pair_type& x, const pair_type& y, size_t k) const {
    return y.second? std::make_pair(y.first*Tp(k), true) : x;
  }
};

int main() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);

  segment_tree<std::pair<int, bool>, find<int>, update<int>> st(n);
  for (size_t i = 0; i < q; ++i) {
    int com;
    scanf("%d", &com);

    if (com == 0) {
      // update(s, t, x)
      size_t s, t;
      int x;
      scanf("%zu %zu %d", &s, &t, &x);
      st.update(s, t+1, {x, true});
    } else if (com == 1) {
      // find(s, t)
      size_t s, t;
      scanf("%zu %zu", &s, &t);
      printf("%d\n", st.aggregate(s, t+1).first);
    }
  }
}
