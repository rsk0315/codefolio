#include <cstdio>
#include <cstdint>
#include <cassert>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>
#include <numeric>
#include <random>

#include "../../DataStructure/basic_segment_tree.cpp"
#include "../DST/disjoint_sparse_table.cpp"

std::mt19937 rsk(0315);

void random_test(size_t n) {
  std::vector<intmax_t> a(n);

  std::uniform_int_distribution<intmax_t> neko(0, 65535);
  for (size_t i = 0; i < n; ++i) a[i] = neko(rsk);

  disjoint_sparse_table<intmax_t> dst(a.begin(), a.end());
  basic_segment_tree<range_sum_single_add<intmax_t>> bst(a.begin(), a.end());
  for (size_t jl = 0; jl < n; ++jl)
    for (size_t jr = jl; jr <= n; ++jr) {
      // int expected = std::accumulate(a.begin()+jl, a.begin()+jr, 0);
      int expected = bst.accumulate(jl, jr);
      int got = dst.accumulate(jl, jr);
      // fprintf(stderr, "[%zu, %zu); expected: %d, got: %d\n",
      //         jl, jr, expected, got);
      assert(expected == got);
    }
}

int main() {
  for (size_t i = 1; i <= 16; ++i) {
    std::vector<int> a(i);
    for (size_t j = 0; j < i; ++j) a[j] = 1 << j;
    disjoint_sparse_table<int> dst(a.begin(), a.end());
    fprintf(stderr, "---\n");

    for (size_t jl = 0; jl < i; ++jl)
      for (size_t jr = jl; jr <= i; ++jr) {
        int expected = std::accumulate(a.begin()+jl, a.begin()+jr, 0);
        int got = dst.accumulate(jl, jr);
        fprintf(stderr, "[%zu, %zu); expected: %d, got: %d\n",
                jl, jr, expected, got);
        assert(expected == got);
      }
  }

  for (size_t i = 1; i <= 10000; ++i) {
    fprintf(stderr, "\r%zu", i);
    fflush(stderr);
    std::uniform_int_distribution<size_t> neko(1024, 4095);
    random_test(neko(rsk));
  }
}
