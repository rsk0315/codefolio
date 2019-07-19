#include <cstdio>
#include <cstdint>
#include <cassert>
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

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   for (size_t j = 0; j < 3; ++j)
  //     printf("%+2.3f%c", a[i][j], j<2? ' ': '\n');

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%+2.3f%c", b[i], i<2? ' ': '\n');

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%+2.3f%c", c[i], i<2? ' ': '\n');

  // puts("");
  // printf("%+2.3f\n", v);

  // for (size_t i = 0; i < 3; ++i)
  //   printf("%zu%c", ni[i], i<2? ' ': '\n');
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%zu%c", bi[i], i<2? ' ': '\n');

  // linear_program<double>::S_pivot(ni, bi, a, b, c, v, 2, 0);

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   for (size_t j = 0; j < 3; ++j)
  //     printf("%+2.3f%c", a[i][j], j<2? ' ': '\n');

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%+2.3f%c", b[i], i<2? ' ': '\n');

  // puts("");
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%+2.3f%c", c[i], i<2? ' ': '\n');

  // puts("");
  // printf("%+2.3f\n", v);

  // for (size_t i = 0; i < 3; ++i)
  //   printf("%zu%c", ni[i], i<2? ' ': '\n');
  // for (size_t i = 0; i < 3; ++i)
  //   printf("%zu%c", bi[i], i<2? ' ': '\n');

  // double y = linear_program<double>::maximize(a, b, c);
  // printf("%.12f\n", y);

  if (false) {
    std::vector<std::vector<double>> a{{-0.2, -0.2},
                                       {1.8, -0.2}};

    std::vector<double> b{0.8, 2.8}, c{1.8, -0.2};

    double v = linear_program<double>::maximize(a, b, c);
    printf("%.12f\n", v);
  }
  if (false) {
    std::vector<std::vector<double>> a{{2.0, -1.0},
                                       {1.0, -5.0}};

    std::vector<double> b{2.0, -4.0}, c{2.0, -1.0};

    double v = linear_program<double>::maximize(a, b, c);
    printf("%.12f\n", v);
  }
  {
    std::vector<std::vector<double>> a{{1.0, 2.0, 1.0},
                                       {3.0, 0.0, 2.0},
                                       {1.0, 4.0, 0.0}};

    std::vector<double> b{430.0*3, 460.0*3, 420.0*3}, c{1.0, 0.67, 1.67};

    double v = linear_program<double>::maximize(a, b, c);
    printf("%.12f\n", v);
  }
}
