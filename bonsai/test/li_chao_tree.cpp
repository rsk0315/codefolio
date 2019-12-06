#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>
#include <limits>
#include <tuple>
#include <numeric>

#include "../../DataStructure/li_chao_tree.cpp"

int main() {
  size_t n;
  intmax_t c;
  scanf("%zu %jd", &n, &c);

  std::vector<intmax_t> h(n);
  for (auto& hi: h) scanf("%jd", &hi);

  intmax_t inf = 1e18;
  std::vector<intmax_t> dp(n, inf);
  li_chao_tree<intmax_t> lct(h.begin(), h.end());

  dp[0] = 0;
  for (size_t i = 1; i < n; ++i) {
    size_t j = i-1;
    intmax_t a = -2*h[j];
    intmax_t b = (dp[j] + h[j]*h[j]);
    lct.push(a, b);
    dp[i] = lct.get(i) + h[i]*h[i] + c;
  }

  printf("%jd\n", dp[n-1]);
}
