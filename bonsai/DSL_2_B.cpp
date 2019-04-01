#define _GLIBCXX_DEBUG
#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>

template <class Tp, class BinaryOperation1, class BinaryOperation2>
class segment_tree {
public:
  using binary_operation1 = BinaryOperation1;
  using binary_operation2 = BinaryOperation2;
  using value_type = Tp;

private:
  size_t m;
  std::vector<value_type> c;
  binary_operation1 op1;  // aggregate
  binary_operation2 op2;  // update

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*n), c(m, op1.identity), op1(op1), op2(op2) {}

  segment_tree(size_t n, const value_type& e,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*n), c(m, e), op1(op1), op2(op2)
  {
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()):
    m(2*std::distance(first, last)), c(m), op1(op1), op2(op2)
  {
    static_assert(std::is_same<Tp, typename ForwardIt::value_type>::value, "");
    for (size_t i = m/2; first != last; ++i) c[i] = *first++;
    init_aggregate();
  }

  void update(size_t i, Tp x) {
    i += m/2;
    c[i] = op2(c[i], x);
    while (i > 1) {
      i >>= 1;
      c[i] = op1(c[i<<1], c[i<<1|1]);
    }
  }

  value_type aggregate(size_t l, size_t r) const {
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
struct add {
  Tp identity = 0;
  Tp operator ()(const Tp& x, const Tp& y) const { return x + y; }
};

int main() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);

  segment_tree<int, add<int>, add<int>> st(n);
  for (size_t i = 0; i < q; ++i) {
    int com;
    scanf("%d", &com);

    if (com == 0) {
      // add(i, x)
      size_t i;
      int x;
      scanf("%zu %d", &i, &x);
      --i;
      st.update(i, x);
    } else if (com == 1) {
      // getSum(s, t)
      size_t s, t;
      scanf("%zu %zu", &s, &t);
      --s;
      --t;
      printf("%d\n", st.aggregate(s, t+1));
    }
  }
}
