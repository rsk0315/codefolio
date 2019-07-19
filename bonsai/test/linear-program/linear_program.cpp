#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <utility>
#include <limits>

#include "../../../Math/linear_program.cpp"

int testcase_ends() {
  size_t n, m;
  if (scanf("%zu %zu", &n, &m) == EOF) return 1;

  std::vector<double> c(n);
  for (auto& ci: c) scanf("%lf", &ci);

  std::vector<std::vector<double>> a(m, std::vector<double>(n));
  std::vector<double> b(m);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = 0; j < n; ++j) scanf("%lf", &a[i][j]);
    scanf("%lf", &b[i]);
    b[i] *= m;
  }

  double y = linear_program<double>::maximize(a, b, c);
  printf("Nasa can spend %.0f taka.\n", std::ceil(y));
  return 0;
}

int main() {
  while (!testcase_ends()) {}
}
