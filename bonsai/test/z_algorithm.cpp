#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <deque>
#include <iostream>

#include "../../DataStructure/z_algorithm.cpp"

void test(std::string const& pat, std::string const& text,
          std::vector<ptrdiff_t> const& expected) {

  // z_array za(pat.begin(), pat.end());
  auto za = make_z_array(pat.begin(), pat.end());

  auto it = std::search(text.begin(), text.end(), za);
  // auto it = std::search(text.begin(), text.end(), std::default_searcher(pat.begin(), pat.end()));
  if (it != text.end()) {
    printf("%td (%s)\n", it-text.begin(), std::string(it, it+pat.length()).c_str());
    assert(it-text.begin() == expected[0]);
  } else {
    printf("not found\n");
    assert(expected.empty());
  }

  std::vector<ptrdiff_t> idx;
  for (auto its: za.search_all(text.begin(), text.end())) {
    std::string s(its.first, its.second);
    printf("%s (%td)\n", s.c_str(), its.first-text.begin());
    idx.push_back(its.first-text.begin());
  }
  assert(idx == expected);
}

void testset() {
  test("abac", "ababacabac", {2, 6});
  test("aaaa", "aaaaaaaaaabaaaaab", {0, 1, 2, 3, 4, 5, 6, 11, 12});
  test("abab", "abababcababab", {0, 2, 7, 9});
  test("aabaabaaa", "aabaabaabaabaaabaabaaaaa", {6, 13});
  test("xxx", "a", {});
  test("xxxxxx", "xx", {});
  test("aab", "abaacabaa", {});
  test("", "xyzqw", {0, 1, 2, 3, 4, 5});
}

int main() {
  testset();
}
