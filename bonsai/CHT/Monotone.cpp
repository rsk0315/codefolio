#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <type_traits>
#include <limits>
#include <tuple>
#include <deque>

template <typename Tp>
class lines {
  // assuming monotonicity
  std::deque<std::pair<Tp, Tp>> lines;

public:
  void append(Tp a, Tp b) {
    if (lines.empty()) {
      lines.emplace_back(a, b);
      return;
    }

    Tp a0, b0, a1, b1;
    std::tie(a1, b1) = lines.back();
    while (lines.size() > 1) {
      std::tie(a0, b0) = lines[lines.size()-2];
      if ((a1-a)*(b1-b0) < (a0-a1)*(b-b1)) break;
      lines.pop_back();
      a1 = a0;
      b1 = b0;
    }
    lines.emplace_back(a, b);
  }

  Tp min_at(Tp x) {
    Tp a0, b0, a1, b1;
    std::tie(a0, b0) = lines[0];
    while (lines.size() > 1) {
      std::tie(a1, b1) = lines[1];
      if (a0*x+b0 < a1*x+b1) break;
      lines.pop_front();
      a0 = a1;
      b0 = b1;
    }
    Tp a, b;
    std::tie(a, b) = lines[0];
    return a*x + b;
  }
};

int main() {
  size_t N;
  scanf("%zu", &N);

  std::vector<intmax_t> a(N);
  for (auto& ai: a) scanf("%jd", &ai);

  lines<intmax_t> cht;
  for (size_t i = 0; i < N; ++i) {
    intmax_t j = i+1;
    cht.append(-2*j, j*j+a[i]);
  }

  for (size_t i = 0; i < N; ++i) {
    intmax_t j = i+1;
    printf("%jd\n", cht.min_at(j)+j*j);
  }
}
