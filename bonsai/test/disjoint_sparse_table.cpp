#include <cstdio>
#include <cstdint>
#include <cassert>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>
#include <numeric>

#include "../DST/disjoint_sparse_table.cpp"

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
}
