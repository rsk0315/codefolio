template <class Tp,
          class BinaryOperation1 = std::plus<Tp>,
          class BinaryOperation2 = std::plus<Tp>>
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
  value_type e1;  // unit elements

  void init_resize(size_t n) {
    while (m < n+n) m <<= 1;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n, const value_type& e,
               const value_type& e1): e1(e1) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               const value_type& e1): e1(e1) {

    static_assert(std::is_same<Tp, typename ForwardIt::value_type>::value, "");
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
    value_type resl = e1;
    value_type resr = e1;
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
