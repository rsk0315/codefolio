#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <deque>

template <class RandomIt, class OutputIt>
OutputIt slide_min(RandomIt first, RandomIt last, ptrdiff_t width, OutputIt d_first) {
  std::deque<RandomIt> st;
  for (auto it = first; it != last; ++it) {
    while (!st.empty() && !(*st.back() < *it)) st.pop_back();
    st.push_back(it);
    if (width <= it - st.front()) st.pop_front();
    if (width-1 <= it - first) *d_first++ = *st.front();
  }
  return d_first;
}

int main() {
  size_t N, L;
  scanf("%zu %zu", &N, &L);

  std::vector<int> a(N);
  for (auto& ai: a) scanf("%d", &ai);

  std::vector<int> sm(N-L+1);
  slide_min(a.begin(), a.end(), L, sm.begin());

  for (size_t i = 0; i <= N-L; ++i)
    printf("%d%c", sm[i], i<N-L? ' ':'\n');
}
