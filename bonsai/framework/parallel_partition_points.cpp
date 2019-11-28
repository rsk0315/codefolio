#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>
#include <tuple>

#include "../../DataStructure/union_find.cpp"
#include "../../framework/parallel_binary_search.cpp"

class neko {
private:
  using edge = std::pair<size_t, size_t>;
  using query = std::tuple<size_t, size_t, size_t>;
  std::vector<edge> M_es;
  disjoint_set M_ds;
  typename std::vector<edge>::iterator M_it;

public:
  neko() = default;
  neko(neko const&) = default;
  neko(neko&&) = default;

  neko(size_t n, std::vector<edge> const& es):
    M_es(es), M_ds(n), M_it(M_es.begin()) {}

  size_t size() const noexcept {
    return M_es.size();
  }

  void proceed() {
    size_t s, t;
    std::tie(s, t) = *M_it++;
    M_ds.unite(s, t);
  }

  void reset() {
    M_ds.reset();
    M_it = M_es.begin();
  }

  bool predicate(query const& q) const {
    size_t x, y, z;
    std::tie(x, y, z) = q;

    size_t w = M_ds.size(x);
    if (!M_ds.equivalent(x, y)) w += M_ds.size(y);
    return w < z;
  }
};

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  std::vector<std::pair<size_t, size_t>> es(m);
  for (auto& p: es) {
    size_t a, b;
    scanf("%zu %zu", &a, &b);
    --a, --b;
    p = std::make_pair(a, b);
  }

  size_t q;
  scanf("%zu", &q);

  std::vector<std::tuple<size_t, size_t, size_t>> qs(q);
  for (auto& t: qs) {
    size_t x, y, z;
    scanf("%zu %zu %zu", &x, &y, &z);
    --x, --y;
    t = std::make_tuple(x, y, z);
  }

  neko s(n, es);
  auto res = parallel_partition_points(s, qs.begin(), qs.end());
  for (auto r: res) printf("%zu\n", r);
}
