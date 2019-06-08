#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>
#include <set>
#include <tuple>
#include <queue>

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

class union_find {
  mutable std::vector<intmax_t> c;

public:
  union_find(size_t n): c(n, -1) {}

  size_t find(size_t v) const {
    if (c[v] < 0)
      return v;

    return (c[v] = find(c[v]));
  }

  bool unite(size_t u, size_t v) {
    u = find(u);
    v = find(v);
    if (u == v)
      return false;

    if (-c[u] > -c[v])
      std::swap(u, v);

    c[v] += c[u];
    c[u] = v;
    return true;
  }

  bool connected(size_t u, size_t v) const {
    return (find(u) == find(v));
  }

  size_t size() const { return c.size(); }
  size_t size(size_t v) const {
    return -c[find(v)];
  }
};

template <typename Weight>
std::pair<graph<Weight>, std::vector<size_t>> biconnected_components(
    graph<Weight> const& g,
    std::vector<edge<Weight>> const& bridge) {

  std::set<std::pair<size_t, size_t>> es;
  for (auto const& e: bridge) {
    es.insert(std::minmax(e.src, e.dst));
  }

  size_t n = g.size();
  union_find uf(n);
  size_t m = n;
  for (size_t v = 0; v < n; ++v)
    for (auto const& e: g[v])
      if (!es.count(std::minmax(e.dst, e.src)))
        if (uf.unite(e.src, e.dst)) --m;

  std::vector<size_t> root(n, -1);
  for (size_t v = 0, u = 0; v < n; ++v) {
    if (uf.find(v) != v) continue;
    root[v] = u++;
  }

  std::vector<size_t> cmp(n);
  for (size_t v = 0; v < n; ++v)
    cmp[v] = root[uf.find(v)];

  graph<Weight> h(m);
  for (auto const& e: bridge)
    h.connect_with(cmp[e.src], cmp[e.dst], e.cost);

  return {h, cmp};
}

template <typename Weight>
std::pair<graph<int>, std::vector<size_t>> biconnected_components(
    graph<Weight> const& g,
    std::vector<size_t> const& articulation_point) {

  size_t n = g.size();
  std::vector<bool> ap(n);
  for (auto v: articulation_point) ap[v] = true;
  
}

template <
  typename Monoid,
  typename Container = std::vector<typename Monoid::first_type>
>
class basic_segment_tree {
public:
  using first_type = typename Monoid::first_type;
  using second_type = typename Monoid::second_type;
  using value_type = first_type;
  using binary_operation = typename Monoid::binary_operation;
  using external_binary_operation = typename Monoid::external_binary_operation;
  using container = Container;

private:
  size_t M_base_size;
  binary_operation M_op1;
  external_binary_operation M_op2;
  container M_c;

public:
  basic_segment_tree() = default;
  basic_segment_tree(basic_segment_tree const&) = default;
  basic_segment_tree(basic_segment_tree&&) = default;
  basic_segment_tree& operator =(basic_segment_tree const&) = default;
  basic_segment_tree& operator =(basic_segment_tree&&) = default;

  basic_segment_tree(size_t n, first_type const& x = binary_operation().identity):
    M_base_size(n),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(n+n, x)
  {
    for (size_t i = n; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  template <typename InputIt>
  basic_segment_tree(InputIt first, InputIt last):
    M_base_size(std::distance(first, last)),
    M_op1(binary_operation()),
    M_op2(external_binary_operation()),
    M_c(M_base_size*2)
  {
    for (size_t i = M_base_size; first != last; ++i)
      M_c[i] = *first++;
    for (size_t i = M_base_size; i--;)
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
  }

  void modify(size_t i, second_type const& x) {
    i += M_base_size;
    M_c[i] = M_op2(M_c[i], x);
    while (i > 1) {
      i >>= 1;
      M_c[i] = M_op1(M_c[i<<1|0], M_c[i<<1|1]);
    }
  }

  first_type accumulate(size_t l, size_t r) const {
    first_type resl = M_op1.identity;
    first_type resr = resl;
    l += M_base_size;
    r += M_base_size;
    while (l < r) {
      if (l & 1) resl = M_op1(resl, M_c[l++]);
      if (r & 1) resr = M_op1(M_c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return M_op1(resl, resr);
  }
};

namespace std {
  template <typename T1, typename T2>
  class numeric_limits<std::pair<T1, T2>> {
  public:
    static std::pair<T1, T2> constexpr max() {
      return {std::numeric_limits<T1>::max(), std::numeric_limits<T2>::max()};
    }
  };
}

template <typename Tp>
struct range_min_single_update {
  using first_type = Tp;
  using second_type = Tp;
  struct binary_operation {
    static first_type constexpr identity = std::numeric_limits<Tp>::max();
    first_type operator ()(first_type const& x, first_type const& y) const {
      return std::min(x, y);
    }
  };
  struct external_binary_operation {
    first_type operator ()(first_type const&, second_type const& y) const {
      return y;
    }
  };
};

class lowest_common_ancestor {
  using container = basic_segment_tree<range_min_single_update<std::pair<size_t, size_t>>>;
  container M_st;
  std::vector<size_t> M_index;

  template <typename Weight>
  void M_build(graph<Weight> const& g, size_t s) {
    size_t n = g.size();
    std::vector<std::pair<size_t, size_t>> base;
    M_index.resize(n);
    make_fix_point([&](auto f, size_t v, size_t p, size_t d) -> void {
        M_index[v] = base.size();
        base.emplace_back(d, v);
        for (auto const& e: g[v]) {
          if (e.dst == p) continue;
          f(e.dst, e.src, d+1);
          base.emplace_back(d, v);
        }
        if (g[v].size() == 1) base.emplace_back(d, v);
    })(s, -1, 0);
    M_st = container(base.begin(), base.end());
  }

public:
  lowest_common_ancestor() = default;
  template <typename Weight>
  lowest_common_ancestor(graph<Weight> const& g, size_t s) {
    M_build(g, s);
  }

  size_t get(size_t u, size_t v) const {
    size_t iu, iv;
    std::tie(iu, iv) = std::minmax(M_index[u], M_index[v]);
    return M_st.accumulate(iu, iv+1).second;
  }
};

template <class Weight>
std::vector<Weight> shortest(const graph<Weight>& g, size_t s) {
  // based on Dijkstra algorithm
  Weight inf = std::numeric_limits<Weight>::max();
  std::vector<Weight> res(g.size(), inf);
  using pair_type = std::pair<Weight, size_t>;
  std::priority_queue<pair_type, std::vector<pair_type>, std::greater<>> q;
  res[s] = 0;
  q.emplace(0, s);
  while (!q.empty()) {
    Weight w;
    size_t v;
    std::tie(w, v) = q.top();
    q.pop();
    if (w > res[v]) continue;

    for (const auto& e: g[v]) {
      if (w + e.cost < res[e.dst]) {
        res[e.dst] = w + e.cost;
        q.emplace(res[e.dst], e.dst);
      }
    }
  }
  return res;
}

int main() {
  size_t n;
  scanf("%zu", &n);

  graph<int> g(n);
  for (size_t i = 1; i < n; ++i) {
    size_t s, t;
    scanf("%zu %zu", &s, &t);
    --s;
    --t;
    g.connect_with(s, t);
  }

  lowest_common_ancestor lca(g, 0);
  size_t q;
  scanf("%zu", &q);

  auto dist = shortest(g, 0);
  for (size_t i = 0; i < q; ++i) {
    size_t u, v;
    scanf("%zu %zu", &u, &v);
    --u;
    --v;
    size_t w = lca.get(u, v);

    intmax_t du = dist[u];
    intmax_t dv = dist[v];
    intmax_t dw = dist[w];
    printf("%jd\n", du+dv-2*dw+1);
  }
}
