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

  dual_segment_tree(size_type n, first_type const& x = binary_operation().identity):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, x)
  {
    for (size_type i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
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
    for (size_type i = M_base_size; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  dual_segment_tree& operator =(dual_segment_tree const&) = default;
  dual_segment_tree& operator =(dual_segment_tree&&) = default;

  void modify(size_type l, size_type r, second_type const& x) {
    
  }

  first_type get(size_type i) const {
  }
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
