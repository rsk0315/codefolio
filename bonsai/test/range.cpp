#include <cstdio>
#include <vector>
#include <type_traits>
#include <limits>
#include <iostream>

#include "../../utility/range.cpp"

int main() {
  size_t n = 9;
  std::vector<int> a;
  a.reserve(n);
  for (auto i: range<int>(n)) {
    printf(" %d", i);
    a.push_back(i);
  }  // 0 1 2 3 4 5 6 7 8
  puts("");

  for (auto i: range<size_t>(n-1, -1, -1)) {
    printf(" %d", a[i]);
  }  // 8 7 6 5 4 3 2 1 0
  puts("");

  for (auto i: range<size_t>(n-1, -1, -3)) {
    printf(" %d", a[i]);
  }  // 8 5 2
  puts("");

  for (auto i: range<int>(0, n, 2)) {
    printf(" %d", i);
  }  // 0 2 4 6 8
  puts("");

  for (auto i: range<int>(0, n, 3)) {
    printf(" %d", i);
  }  // 0 3 6
  puts("");
}
