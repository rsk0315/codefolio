template <class Tp, class BinaryOperation1, class BinaryOperation2>
class segment_tree {
public:
  using binary_operation1 = BinaryOperation1;
  using binary_operation2 = BinaryOperation2;
  using value_type = Tp;

private:
  std::vector<value_type> c;
  size_t m = 1;
  binary_operation1 op1;  // aggregate
  binary_operation2 op2;  // update

  void init_resize(size_t n) {
    m = 2*n;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    init_resize(n);
    c.assign(m, op1.identity);
  }

  segment_tree(size_t n, const Tp& e,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    static_assert(std::is_convertible<typename ForwardIt::value_type, Tp>::value, "");
    init_resize(std::distance(first, last));
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
      size_t j = __builtin_ctzll(l | r);
      l >>= j;
      r >>= j;
    }
    return op1(resl, resr);
  }

  value_type operator [](size_t k) const { return c[k+m/2]; }
};
