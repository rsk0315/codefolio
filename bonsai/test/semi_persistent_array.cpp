#include <cstdio>
#include <algorithm>
#include <utility>
#include <vector>
#include <random>

#include "../../DataStructure/semi_persistent_array.cpp"

int main() {
  size_t n = 10;
  semi_persistent_array<int> spa(n);
  size_t t = 10;
  std::mt19937 rsk(0315);
  std::uniform_int_distribution<size_t> neko(0, n-1);
  std::uniform_int_distribution<int> nya(0, 99);  
  for (size_t i = 0; i < t; ++i) {
    size_t j = neko(rsk);
    int x = nya(rsk);
    fprintf(stderr, "[%zu]: %d\n", j, x);
    spa.set(j, x);
  }

  for (size_t i = 0; i < t; ++i)
    for (size_t j = 0; j < n; ++j)
      printf("%2d%c", spa.get(j, i), j+1<n? ' ':'\n');
}
