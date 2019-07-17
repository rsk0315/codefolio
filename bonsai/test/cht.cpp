#include <cstdio>
#include <cstdint>
#include <map>
#include <vector>
#include <utility>
#include <tuple>
#include <limits>

#include "../../DataStructure/cht.cpp"

int main() {
  size_t n;
  scanf("%zu", &n);

  std::vector<intmax_t> a(n);
  for (auto& ai: a) scanf("%jd", &ai);

  linear_minima<intmax_t> lm;
  for (size_t i = 0; i < n; ++i) {
    intmax_t j = i+1;
    lm.append(-2*j, j*j+a[i]);
  }

  for (size_t i = 0; i < n; ++i) {
    intmax_t j = i+1;
    printf("%jd\n", lm.min_at(j)+j*j);
  }
}
