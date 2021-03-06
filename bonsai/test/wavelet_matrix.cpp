#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <climits>
#include <vector>
#include <algorithm>
#include <utility>
#include <array>

constexpr intmax_t  operator ""_jd(unsigned long long n) { return n; }
constexpr uintmax_t operator ""_ju(unsigned long long n) { return n; }
constexpr size_t    operator ""_zu(unsigned long long n) { return n; }
// constexpr ptrdiff_t operator ""_td(unsigned long long n) { return n; }

#include "../WM/bit_vector.cpp"

#include <cassert>
#include <set>
#include <random>
#include <string>

std::mt19937 rsk(0315);

void random_test() {
  size_t n = (1 << 18) - 1;
  std::vector<bool> vb(n);
  // neko の初期値で 0/1 の割合を決めて，
  // if (neko(rsk)) を ! するかどうかで
  // どちらを 1 に割り振るかを決める．
  std::uniform_int_distribution<int> neko(0, 2);
  std::set<size_t> select;
  std::vector<size_t> rank(n+1);
  for (size_t i = 0; i < n; ++i) {
    rank[i+1] = rank[i];
    if (neko(rsk)) continue;
    // sparse 部分のチェック
    if (10000 <= i && i <= 16000) continue;
    vb[i] = true;
    select.insert(i);
    ++rank[i+1];
  }

  // for (size_t i = 0; i < n; ++i)
  //   fprintf(stderr, "%d%c", !!vb[i], i+1<n? ' ': '\n');

  bit_vector bv(vb.begin(), vb.end());
  if (true) {
    for (size_t i = 0; i <= n; ++i) {
      // fprintf(stderr, "rank[%zu]: %zu, bv.rank1(0, %zu): %zu\n",
      //         i, rank[i], i, bv.rank1(0, i));
      assert(rank[i] == bv.rank1(0, i));
    }
  }

  if (true) {
    size_t i = 1;
    for (auto s: select) {
      size_t t = bv.select1(i, 0);
      fprintf(stderr, "select[%zu]: %zu, bv.select1(%zu): %zu\n",
              i, s+1, i, t);
      ++i;
      // if (i == 1000) break;
      assert(s+1 == t);
    }
  }
}

int main() {
  random_test();
}
