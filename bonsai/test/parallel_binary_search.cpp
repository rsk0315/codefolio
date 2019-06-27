#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>

#include "../../DataStructure/union_find.cpp"

std::vector<size_t> parallel_binary_search(size_t n,
                                           std::vector<std::pair<size_t, size_t>> const& ab,
                                           std::vector<std::pair<size_t, size_t>> const& xy) {

  size_t m = ab.size();
  size_t q = xy.size();
  std::vector<size_t> lb(q), ub(q, m+1);
  while (true) {
    bool decided = true;
    std::vector<std::vector<size_t>> ev(m+1);
    for (size_t j = 0; j < q; ++j) {
      if (ub[j]-lb[j] > 1) {
        decided = false;
        size_t mid = (lb[j]+ub[j]) >> 1;
        ev[mid].push_back(j);
      }
    }
    if (decided) break;

    union_find uf(n);
    for (size_t i = 0; i <= m; ++i) {
      while (!ev[i].empty()) {
        size_t j = ev[i].back();
        ev[i].pop_back();
        size_t x, y;
        std::tie(x, y) = xy[j];
        (uf.connected(x, y)? ub[j] : lb[j]) = i;
      }
      if (i < m) uf.unite(ab[i].first, ab[i].second);
    }
  }
  return lb;
}

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  std::vector<std::pair<size_t, size_t>> ab(m);
  for (auto& p: ab) {
    scanf("%zu %zu", &p.first, &p.second);
    --p.first;
    --p.second;
  }

  size_t q;
  scanf("%zu", &q);

  std::vector<std::pair<size_t, size_t>> xy(q);
  for (auto& p: xy) {
    scanf("%zu %zu", &p.first, &p.second);
    --p.first;
    --p.second;
  }

  auto res = parallel_binary_search(n, ab, xy);
  for (auto ri: res) {
    if (ri < m) {
      printf("%zu\n", ri+1);
    } else {
      puts("-1");
    }
  }
}
