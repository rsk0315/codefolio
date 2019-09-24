#include <cstdio>
#include <cstdint>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <memory>

constexpr intmax_t  operator ""_jd(unsigned long long n) { return n; }
constexpr uintmax_t operator ""_ju(unsigned long long n) { return n; }
constexpr size_t    operator ""_zu(unsigned long long n) { return n; }
// constexpr ptrdiff_t operator ""_td(unsigned long long n) { return n; }

#include "../../DataStructure/x_fast_trie.cpp"

template <typename Key, typename Tp>
using x_hash = std::unordered_map<Key, Tp>;

int main() {
  x_fast_trie<3, x_hash> xft;
  // xft.insert(3);
  // xft.inspect();

  // xft.insert(6);
  // xft.inspect();

  // xft.insert(8);
  // xft.inspect();

  auto ins = [&](int x) { xft.insert(x); xft.inspect(); };
  auto ers = [&](int x) { xft.erase(x); xft.inspect(); };
  // // ins(1);
  // ins(4);
  // // ins(5);
  // ers(4);
  ins(1);
  ins(2);
  ins(3);
  ers(2);
}
