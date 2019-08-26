template <
  typename Monoid,
  typename Container = std::vector<typename Monoid::first_type>
>
class basic_segment_tree {
public:
  using size_type = size_t;
  using first_type = typename Monoid::first_type;
  using second_type = typename Monoid::second_type;
  using value_type = first_type;
  using binary_operation = typename Monoid::binary_operation;
  using external_binary_operation = typename Monoid::external_binary_operation;
  using container = Container;

private:
  size_type M_base_size;
  binary_operation M_op1;
  external_binary_operation M_op2;
  container M_c;

  template <typename Predicate>
  size_type M_search_root(Predicate pred, value_type& x) const {
    size_type n = M_base_size;
    size_type l = n;
    size_type r = n+n;
    size_type v = r;
    std::vector<size_type> rs;
    x = M_op1.identity;
    while (l < r) {
      if (l & 1) {
        if (!pred(M_op1(x, M_c[l]))) return l;
        x = M_op1(x, M_c[l++]);
      }
      if (r & 1) rs.push_back(--r);
      l >>= 1;
      r >>= 1;
    }
    while (!rs.empty()) {
      size_type r = rs.back();
      rs.pop_back();
      if (!pred(M_op1(x, M_c[r]))) return r;
      x = M_op1(x, M_c[r]);
    }
    return v;
  }

  template <typename Predicate>
  size_type M_search_leaf(Predicate pred, size_type v, value_type& x) const {
    size_type n = M_base_size;
    while (v < n) {
      size_type c = v << 1;
      if (pred(M_op1(x, M_c[c]))) {
        x = M_op1(x, M_c[c]);
        c |= 1;
      }
      v = c;
    }
    return v - n;
  }

public:
  basic_segment_tree() = default;
  basic_segment_tree(basic_segment_tree const&) = default;
  basic_segment_tree(basic_segment_tree&&) = default;

  basic_segment_tree(size_type n, first_type const& x = binary_operation().identity):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, x)
  {
    for (size_type i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  template <typename InputIt>
  basic_segment_tree(InputIt first, InputIt last):
    M_op1(binary_operation()), M_op2(external_binary_operation())
  { assign(first, last); }

  basic_segment_tree& operator =(basic_segment_tree const&) = default;
  basic_segment_tree& operator =(basic_segment_tree&&) = default;

  void assign(size_type n, value_type const& x) {
    M_base_size = n;
    M_c.assign(n+n, x);
    for (size_type i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }
  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    container tmp(first, last);
    M_base_size = tmp;
    M_c.assign(M_base_size);
    M_c.insert(M_c.end(), tmp.begin(), tmp.end());
    for (size_type i = M_base_size; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  void modify(size_type i, second_type const& x) {
    i += M_base_size;
    M_c[i] = M_op2(M_c[i], x);
    while (i > 1) {
      i >>= 1;
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
    }
  }

  void assign_at(size_type i, value_type const& x) {
    i += M_base_size;
    M_c[i] = x;
    while (i > 1) {
      i >>= 1;
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
    }
  }

  value_type const& operator [](size_type i) const { return M_c[i + M_base_size]; }

  value_type accumulate(size_type l, size_type r) {
    first_type resl = M_op1.identity;
    first_type resr = resl;
    l += M_base_size;
    r += M_base_size;
    while (l < r) {
      if (l & 1) resl = M_op1(resl, M_c[l++]);
      if (r & 1) resr = M_op1(M_c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return M_op1(resl, resr);
  }

  template <typename Predicate>
  std::pair<size_type, value_type> partition_point(Predicate pred) const {
    value_type value;
    size_type root = M_search_root(pred, value);
    size_type bound = M_search_leaf(pred, root, value);
    return {bound, value};
  }
};

template <typename Tp>
struct range_sum_single_add {
  using first_type = Tp;
  using second_type = Tp;
  struct binary_operation {
    first_type identity{};
    first_type operator ()(first_type const& x, first_type const& y) const {
      return x + y;
    }
  };
  struct external_binary_operation {
    first_type operator ()(first_type const& x, second_type const& y) const {
      return x + y;
    }
  };
};
