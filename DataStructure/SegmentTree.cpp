template <class Tp,
          class BinaryOperation1 = std::plus<Tp>,
          class BinaryOperation2 = std::plus<Tp>>
class segment_tree {
  std::vector<Tp> c;
  size_t m = 1;
  BinaryOperation1 op1;  // update
  BinaryOperation2 op2;  // aggregate
  Tp e2;  // unit elements

  void init_resize(size_t n) {
    while (m <= n) m <<= 1;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op2(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n, Tp e=Tp(),
               const BinaryOperation1& op1=std::plus<Tp>(),
               const BinaryOperation2& op2=std::plus<Tp>(),
               const Tp& e2=0): op1(op1), op2(op2), e2(e2) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               const BinaryOperation1& op1=std::plus<Tp>(),
               const BinaryOperation2& op2=std::plus<Tp>(),
               const Tp& e2=0): op1(op1), op2(op2), e2(e2) {

    static_assert(std::is_same<Tp, typename ForwardIt::value_type>::value, "");
    init_resize(std::distance(first, last));
    for (size_t i = m/2; first != last; ++i) c[i] = *first++;
    init_aggregate();
  }

  void update(size_t i, Tp x) {
    i += m/2;
    c[i] = op1(c[i], x);
    while (i > 1) {
      i >>= 1;
      c[i] = op2(c[i<<1], c[i<<1|1]);
    }
  }

  Tp aggregate(size_t l, size_t r) const {
    Tp resl = e2;
    Tp resr = e2;
    l += m/2;
    r += m/2;
    while (l < r) {
      if (l & 1) resl = op2(resl, c[l++]);
      if (r & 1) resr = op2(c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return op2(resl, resr);
  }
};
