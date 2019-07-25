#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <deque>
#include <utility>
#include <algorithm>

template <typename RandomIt, typename OutputIt>
OutputIt window_minima(RandomIt first, RandomIt s_last, RandomIt last, OutputIt d_first) {
  std::deque<RandomIt> dq;
  ptrdiff_t width = std::distance(first, s_last);
  for (auto it = first; it < last; ++it) {
    while (!dq.empty() && *it < *dq.back()) dq.pop_back();
    dq.push_back(it);
    if (width <= it - dq.front()) dq.pop_front();
    if (width-1 <= it - first) *d_first++ = *dq.front();
  }
  return d_first;
}

int main() {
  size_t N;
  ptrdiff_t L;
  scanf("%zu %td", &N, &L);

  std::vector<int> a(N);
  for (auto& ai: a) scanf("%d", &ai);

  std::vector<int> res(N-L+1);
  window_minima(a.begin(), a.begin()+L, a.end(), res.begin());
  for (size_t i = 0; i <= N-L; ++i)
    printf("%d%c", res[i], i<N-L? ' ': '\n');
}


/*
1 7 7 4 8 1 6
1 
1 7
1 7 7
- # # 4
- - # 4 8
- - - # # 1
- - - - # 1 6

9 7 7 4 8 5 6
9
# 7
# 7 7
- # # 4
- - # 4 8
- - - 4 # 5
- - - - # 5 6
 */
