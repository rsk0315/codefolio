template <
  typename Monoid,
  typename Container = std::vector<typename Monoid::first_type>,
  typename SubContainer = std::vector<typename Monoid::second_type>
>
class segment_tree {
public:
  using first_type = typename Monoid::first_type;
  using second_type = typename Monoid::second_type;
  using value_type = first_type;
  using binary_operation = typename Monoid::binary_operation;
  using external_binary_operation = typename Monoid::external_binary_operation;
  using merge_operation = typename Monoid::merge_operation;
  using container = Container;
  using sub_container = SubContainer;

private:
  size_t M_base_size;
  binary_operation M_op1;
  external_binary_operation M_op2;
  merge_operation M_op3;
  container M_c;
  sub_container M_d;  // deferred

  void M_build(size_t i) {
    while (i > 1) {
      i >>= 1;
      M_c[i] = M_op2(M_op1(M_c[i<<1|0], M_c[i<<1|1]), M_d[i]);
    }
  }

  void M_resolve(size_t i) {
    size_t h = (sizeof(long long) * CHAR_BIT) - __builtin_clzll(M_base_size*2);
    for (size_t s = h; s > 0; --s) {
      size_t p = i >> s;
      if (M_d[p] != M_op3.identity) {
        M_apply(p<<1|0, M_d[p]);
        M_apply(p<<1|1, M_d[p]);
        M_d[p] = M_op3.identity;
      }
    }
  }

  void M_apply(size_t i, second_type const& x) {
    M_c[i] = M_op2(M_c[i], x);
    if (i < M_base_size) M_d[i] = M_op3(M_d[i], x);
  }

public:
  segment_tree() = default;
  segment_tree(segment_tree const&) = default;
  segment_tree(segment_tree&&) = default;

  segment_tree(size_t n, first_type const& x = binary_operation().identity):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_op3(merge_operation()),
    M_c(n+n, x), M_d(n, M_op3.identity)
  {
    for (size_t i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  template <typename InputIt>
  segment_tree(InputIt first, InputIt last):
    M_base_size(std::distance(first, last)),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_op3(merge_operation()),
    M_c(M_base_size*2), M_d(M_base_size, M_op3.identity)
  {
    for (size_t i = M_base_size; first != last; ++i)
      M_c[i] = *first++;
    for (size_t i = M_base_size; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  segment_tree& operator =(segment_tree const&) = default;
  segment_tree& operator =(segment_tree&&) = default;

  void modify(size_t l, size_t r, second_type const& x) {
    if (l == r) return;  // for [n, n)
    l += M_base_size;
    r += M_base_size;
    size_t l0 = l;
    size_t r0 = r;
    while (l < r) {
      if (l & 1) M_apply(l++, x);
      if (r & 1) M_apply(--r, x);
      l >>= 1;
      r >>= 1;
    }
    M_build(l0);
    M_build(r0-1);
  }

  first_type accumulate(size_t l, size_t r) {
    first_type resl = M_op1.identity;
    first_type resr = resl;
    if (l == r) return resl;  // for [n, n)

    l += M_base_size;
    r += M_base_size;
    M_resolve(l);
    M_resolve(r-1);
    while (l < r) {
      if (l & 1) resl = M_op1(resl, M_c[l++]);
      if (r & 1) resr = M_op1(M_c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return M_op1(resl, resr);
  }

  first_type operator [](size_t i) {
    i += M_base_size;
    M_resolve(i);
    return M_c[i];
  }
};

template <typename Tp>
struct range_sum_range_add {
  using first_type = std::pair<Tp, Tp>;
  using second_type = Tp;
  struct binary_operation {
    first_type identity{0, 1};
    first_type operator ()(first_type const& x, first_type const& y) const {
      return {x.first+y.first, x.second+y.second};
    }
  };
  struct external_binary_operation {
    first_type operator ()(first_type const& x, second_type const& y) const {
      // a+b(x+c) = (a+bc)+bx
      return {x.first+x.second*y, x.second};
    }
  };
  struct merge_operation {
    second_type identity{0};
    second_type operator ()(second_type const& x, second_type const& y) const {
      return x+y;
    }
  };
};
