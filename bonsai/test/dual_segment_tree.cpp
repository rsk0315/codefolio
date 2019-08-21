#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <utility>

#include "../../DataStructure/dual_segment_tree.cpp"

int main() {
  size_t n = 10;
  dual_segment_tree<single_get_range_add<intmax_t>> st(n);

  st.modify(2, 6, 4);
  st.modify(4, 9, 1);
  for (size_t i = 0; i < n; ++i)
    fprintf(stderr, "%jd%c", st.get(i), i+1<n? ' ': '\n');
}
