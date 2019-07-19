#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <utility>
#include <limits>

#define class struct
#define private public
#include "../../Math/linear_program.cpp"

int main() {
  // linear_program<double>::maximize(1, 2, 3);

  std::vector<std::vector<double>> a{{1.0, 1.0, 3.0},
                                     {2.0, 2.0, 5.0},
                                     {4.0, 1.0, 2.0}};

  std::vector<double> b{30.0, 24.0, 36.0}, c{3.0, 1.0, 2.0};
  std::vector<size_t> ni{0, 1, 2}, bi{3, 4, 5};
  double v = 0.0;

  puts("");
  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 3; ++j)
      printf("%+2.3f%c", a[i][j], j<2? ' ': '\n');

  puts("");
  for (size_t i = 0; i < 3; ++i)
    printf("%+2.3f%c", b[i], i<2? ' ': '\n');

  puts("");
  for (size_t i = 0; i < 3; ++i)
    printf("%+2.3f%c", c[i], i<2? ' ': '\n');

  puts("");
  printf("%+2.3f\n", v);

  for (size_t i = 0; i < 3; ++i)
    printf("%zu%c", ni[i], i<2? ' ': '\n');
  for (size_t i = 0; i < 3; ++i)
    printf("%zu%c", bi[i], i<2? ' ': '\n');

  linear_program<double>::S_pivot(ni, bi, a, b, c, v, 2, 0);

  puts("");
  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 3; ++j)
      printf("%+2.3f%c", a[i][j], j<2? ' ': '\n');

  puts("");
  for (size_t i = 0; i < 3; ++i)
    printf("%+2.3f%c", b[i], i<2? ' ': '\n');

  puts("");
  for (size_t i = 0; i < 3; ++i)
    printf("%+2.3f%c", c[i], i<2? ' ': '\n');

  puts("");
  printf("%+2.3f\n", v);

  for (size_t i = 0; i < 3; ++i)
    printf("%zu%c", ni[i], i<2? ' ': '\n');
  for (size_t i = 0; i < 3; ++i)
    printf("%zu%c", bi[i], i<2? ' ': '\n');
}
