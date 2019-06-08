#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>

template <class F>
class fix_point: F {
public:
  explicit constexpr fix_point(F&& f) noexcept: F(std::forward<F>(f)) {}

  template <class... Args>
  constexpr decltype(auto) operator ()(Args&&... args) const {
    return F::operator ()(*this, std::forward<Args>(args)...);
  }
};

template <class F>
static inline constexpr decltype(auto) make_fix_point(F&& f) noexcept {
  return fix_point<F>{std::forward<F>(f)};
}


template <typename Weight>
struct edge {
  using value_type = Weight;
  size_t src, dst;
  value_type cost;

  edge(size_t src, size_t dst, value_type cost = 1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(edge<value_type> const& rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <typename Weight>
class graph: public std::vector<std::vector<edge<Weight>>> {
public:
  using value_type = Weight;

private:
  size_t M_size = 0;
  std::vector<size_t> M_ord, M_low;
  std::vector<size_t> M_articulation_point;
  std::vector<edge<value_type>> M_bridge;

  void M_lowlink_dfs(size_t s) {
    size_t count = 0;
    make_fix_point([&](auto f, size_t v, size_t p) -> void {
        M_low[v] = M_ord[v] = count++;
        size_t deg = 0;  // in terms of the dfs tree
        bool ap = false;
        for (auto const& e: (*this)[v]) {
          if (e.dst == p) continue;
          if (M_ord[e.dst]+1 != 0) {
            M_low[e.src] = std::min(M_low[e.src], M_ord[e.dst]);
          } else {
            f(e.dst, e.src);
            M_low[e.src] = std::min(M_low[e.src], M_low[e.dst]);
            ++deg;
            if (M_ord[e.src] > 0 && M_ord[e.src] <= M_low[e.dst]) ap = true;
            if (M_ord[e.src] < M_low[e.dst]) M_bridge.push_back(e);
          }
        }
        if (M_ord[v] == 0 && deg > 1) ap = true;
        if (ap) M_articulation_point.push_back(v);
    })(s, -1);
  }

  void M_lowlink() {
    if (!M_ord.empty()) return;
    M_ord.resize(M_size, -1);
    M_low.resize(M_size, -1);
    for (size_t i = 0; i < M_size; ++i)
      if (M_ord[i]+1 == 0) M_lowlink_dfs(i);
  }

public:
  graph() = default;
  graph(size_t n): std::vector<std::vector<edge<value_type>>>(n), M_size(n) {}

  void connect_to(size_t src, size_t dst, value_type cost = 1) {
    (*this)[src].emplace_back(src, dst, cost);
  }

  void connect_with(size_t src, size_t dst, value_type cost = 1) {
    connect_to(src, dst, cost);
    connect_to(dst, src, cost);
  }

  std::vector<edge<value_type>> bridge() {
    M_lowlink();
    return M_bridge;
  }

  std::vector<size_t> articulation_point() {
    M_lowlink();
    return M_articulation_point;
  }
};

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  graph<int> g(n);
  for (size_t i = 0; i < m; ++i) {
    size_t s, t;
    scanf("%zu %zu", &s, &t);
    g.connect_with(s, t);
  }

  auto bridge = g.bridge();
  std::vector<std::pair<size_t, size_t>> res;
  for (auto const& e: bridge)
    res.push_back(std::minmax(e.src, e.dst));

  std::sort(res.begin(), res.end());
  for (auto const& e: res)
    printf("%zu %zu\n", e.first, e.second);
}
