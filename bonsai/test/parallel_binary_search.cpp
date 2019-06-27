#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>

#include "../../DataStructure/union_find.cpp"
#include "../../algorithm/parallel_binary_search.cpp"

class oracle {
  size_t n;
  std::vector<std::pair<size_t, size_t>> ab, xy;
  size_t i = 0;
  union_find uf;

public:
  oracle() = default;
  oracle(size_t n,
         std::vector<std::pair<size_t, size_t>> const& ab,
         std::vector<std::pair<size_t, size_t>> const& xy):
    n(n), ab(ab), xy(xy), uf(n)
  {}

  size_t num_modifications() const { return ab.size(); }
  size_t num_queries() const { return xy.size(); }

  void modify() {
    size_t a, b;
    std::tie(a, b) = ab[i++];
    uf.unite(a, b);
  }

  bool query(size_t j) const {
    size_t x, y;
    std::tie(x, y) = xy[j];
    return uf.connected(x, y);
  }

  void reset_state() {
    i = 0;
    uf = union_find(n);
  }
};

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

  oracle orc(n, ab, xy);
  auto res = parallel_binary_search(orc);
  for (auto ri: res) {
    if (ri < m) {
      printf("%zu\n", ri+1);
    } else {
      puts("-1");
    }
  }
}
