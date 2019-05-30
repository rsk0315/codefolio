#include <cstdio>
#include <cstdint>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>

template <
  typename Monoid,
  typename Container = std::vector<typename Monoid::first_type>
>
class simple_segment_tree {
public:
  using first_type = typename Monoid::first_type;
  using second_type = typename Monoid::second_type;
  using value_type = first_type;
  using binary_operation = typename Monoid::binary_operation;
  using external_binary_operation = typename Monoid::external_binary_operation;
  using container = Container;

private:
  size_t M_base_size;
  binary_operation M_op1;
  external_binary_operation M_op2;
  container M_c;

public:
  simple_segment_tree() = default;
  simple_segment_tree(simple_segment_tree const&) = default;
  simple_segment_tree(simple_segment_tree&&) = default;

  simple_segment_tree(size_t n, first_type const& x = binary_operation().identity):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, x)
  {
    for (size_t i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  template <typename InputIt>
  simple_segment_tree(InputIt first, InputIt last):
    M_base_size(std::distance(first, last)),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(M_base_size*2)
  {
    for (size_t i = M_base_size; first != last; ++i)
      M_c[i] = *first++;
    for (size_t i = M_base_size; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  simple_segment_tree& operator =(simple_segment_tree const&) = default;
  simple_segment_tree& operator =(simple_segment_tree&&) = default;

  void modify(size_t i, second_type const& x) {
    i += M_base_size;
    M_c[i] = M_op2(M_c[i], x);
    while (i > 1) {
      i >>= 1;
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
    }
  }

  first_type accumulate(size_t l, size_t r) {
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
};

template <typename Tp>
struct range_min_single_update {
  using first_type = Tp;
  using second_type = Tp;
  struct binary_operation {
    first_type identity = std::numeric_limits<Tp>::max();
    first_type operator ()(first_type const& x, first_type const& y) const {
      return std::min(x, y);
    }
  };
  struct external_binary_operation {
    first_type operator ()(first_type const&, second_type const& y) const {
      return y;
    }
  };
};

int main() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);
  simple_segment_tree<range_min_single_update<int>> st(n);

  for (size_t i = 0; i < q; ++i) {
    int t;
    scanf("%d", &t);

    if (t == 0) {
      // update(j, x)
      size_t j;
      intmax_t x;
      scanf("%zu %jd", &j, &x);
      st.modify(j, x);
    } else if (t == 1) {
      // getSum(s, t)
      size_t s, t;
      scanf("%zu %zu", &s, &t);
      printf("%d\n", st.accumulate(s, t+1));
    }
  }
}
