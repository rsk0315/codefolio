template <
  typename Monoid,
  typename Container = std::vector<typename Monoid::first_type>
>
class dual_segment_tree {
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

public:
  dual_segment_tree() = default;
  dual_segment_tree(dual_segment_tree const&) = default;
  dual_segment_tree(dual_segment_tree&&) = default;

  dual_segment_tree(size_type n):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, M_op1.identity)
  {}

  dual_segment_tree(size_type n, first_type const& x):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, M_op1.identity)
  {
    modify(0, n, x);
  }

  template <typename InputIt>
  dual_segment_tree(InputIt first, InputIt last):
    M_base_size(std::distance(first, last)),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(M_base_size*2)
  {
    for (size_type i = M_base_size; first != last; ++i)
      M_c[i] = *first++;
  }

  dual_segment_tree& operator =(dual_segment_tree const&) = default;
  dual_segment_tree& operator =(dual_segment_tree&&) = default;

  void modify(size_type l, size_type r, second_type const& x) {
    l += M_base_size;
    r += M_base_size;
    while (l < r) {
      if (l & 1) M_c[l] = M_op2(std::move(M_c[l]), x), ++l;
      if (r & 1) --r, M_c[r] = M_op2(std::move(M_c[r]), x);
      l >>= 1;
      r >>= 1;
    }
  }

  value_type get(size_type i) const {
    i += M_base_size;
    value_type res = M_c[i];
    while (i > 1) {
      i >>= 1;
      res = M_op1(std::move(res), M_c[i]);
    }
    return res;
  }

  value_type operator [](size_type i) const { return get(i); }
};

template <typename Tp>
struct single_get_range_add {
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
