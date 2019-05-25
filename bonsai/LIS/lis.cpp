#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

template <typename InputIt, typename OutputIt>
OutputIt longest_increasing(InputIt first, InputIt last, OutputIt d_first) {
  using value_type = typename InputIt::value_type;
  using difference_type = typename InputIt::difference_type;
  value_type inf = std::numeric_limits<value_type>::max();
  difference_type len = std::distance(first, last);
  std::vector<value_type> dp(len+1, inf);
  do {
    auto it = std::lower_bound(dp.begin(), dp.end(), *first);
    if (!(*it < inf)) *d_first++ = *first;
    *it = *first;
  } while (++first != last);
  return d_first;
}

template <typename InputIt>
typename InputIt::difference_type longest_increasing(InputIt first, InputIt last) {
  using value_type = typename InputIt::value_type;
  using difference_type = typename InputIt::difference_type;
  value_type inf = std::numeric_limits<value_type>::max();
  difference_type len = std::distance(first, last);
  std::vector<value_type> dp(len+1, inf);
  difference_type res = 0;
  do {
    auto it = std::lower_bound(dp.begin(), dp.end(), *first);
    if (!(*it < inf)) ++res;
    *it = *first;
  } while (++first != last);
  return res;
}

int main() {
  std::vector<int> a{1, 2, 3, 1, 2, 3, 1, 2, 3};
  std::vector<int> b(a.size());
  auto it = longest_increasing(a.begin(), a.end(), b.begin());
  b.erase(it, b.end());
  for (size_t i = 0; i < b.size(); ++i)
    printf("%d%c", b[i], i+1<b.size()? ' ':'\n');
  printf("%td\n", longest_increasing(a.begin(), a.end()));
}
