#include <cstdio>
#include <algorithm>
#include <utility>
#include <vector>
#include <memory>
#include <queue>

#include <random>

#include "../../DataStructure/fully_persistent_array.cpp"

int main() {
  size_t n = 10;
  fully_persistent_array<int> fpa(n);
  // auto ss0 = fpa.get_snapshot();
  // for (size_t i = 0; i < n; ++i)
  //   printf("%d%c", ss0.get(i), i+1<n? ' ':'\n');

  // auto ss1 = ss0.set(1, 2);
  // for (size_t i = 0; i < n; ++i)
  //   printf("%d%c", ss1.get(i), i+1<n? ' ':'\n');

  std::mt19937 rsk(0315);
  std::uniform_int_distribution<size_t> nya(0, n-1);
  std::uniform_int_distribution<int> neko(0, 99);

  using snapshot = typename fully_persistent_array<int>::snapshot;
  std::vector<snapshot> ss;
  ss.push_back(fpa.get_snapshot());

  size_t t = 20;
  for (size_t i = 0; i < t; ++i) {
    size_t j = nya(rsk);
    int x = neko(rsk);
    ss.push_back(ss.back().set(j, x));
    fprintf(stderr, "[%zu] <- %d\n", j, x);

    for (size_t k = 0; k < n; ++k) {
      printf("%2d%c", ss.back().get(k), k+1<n? ' ': '\n');
    }
  }

  for (size_t i = 0; i <= t; ++i) {
    for (size_t k = 0; k < n; ++k) {
      printf("%2d%c", ss[i].get(k), k+1<n? ' ': '\n');
    }
  }
}
