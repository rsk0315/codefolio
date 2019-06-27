#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>

#include "../../DataStructure/basic_segment_tree.cpp"

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

int main() {
  size_t n = 8;
  basic_segment_tree<range_sum_single_add<int>> st(n);
  st.modify(0, 2);
  st.modify(5, 1);
  for (size_t i = 1; i <= n; ++i)
    fprintf(stderr, "[0, %zu): %d\n", i, st.accumulate(0, i));

  printf("%zu\n", st.bound([](int x) { return x < 3; }));
}
