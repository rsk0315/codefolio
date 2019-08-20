#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <queue>
#include <functional>
#include <utility>

#include "../../algorithm/bucket_sort.cpp"

int main() {
  std::vector<int> a{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
  for (size_t i = 0; i < a.size(); ++i)
    fprintf(stderr, "%d%c", a[i], i+1<a.size()? ' ': '\n');

  bucket_sort(a.begin(), a.end());
  for (size_t i = 0; i < a.size(); ++i)
    fprintf(stderr, "%d%c", a[i], i+1<a.size()? ' ': '\n');
}
