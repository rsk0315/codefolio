#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>
#include <set>
#include <stack>
#include <tuple>

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

template <class Weight>
struct edge {
  using value_type = Weight;
  size_t src, dst;
  value_type cost;

  edge(size_t src, size_t dst, value_type cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const edge<value_type> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
struct graph: public std::vector<std::vector<edge<Weight>>> {
  using value_type = Weight;
  graph(size_t n): std::vector<std::vector<edge<value_type>>>(n) {}

  void connect_with(size_t src, size_t dst, value_type cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
    (*this)[dst].emplace_back(dst, src, cost);
  }

  void connect_to(size_t src, size_t dst, value_type cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
  }

  bool is_undirected_tree() const {
    size_t n = this->size();
    union_find ufl(n), ufg(n);
    size_t el = 0;
    size_t eg = 0;
    for (auto const& v: *this)
      for (auto const& e: v) {
        if (e.src < e.dst) {
          ++eg;
          if (!ufg.unite(e.src, e.dst)) return false;
        } else if (e.src > e.dst) {
          ++el;
          if (!ufl.unite(e.src, e.dst)) return false;
        } else {
          return false;
        }
      }
    return (el == n-1 && eg == n-1);
  }
};

template <class InputIt>
auto min_spanning_tree(size_t v, InputIt first, InputIt last) {
  // return type: std::pair<Weight, std::set<edge<Weight>>
  // based on Kruskal algorithm
  using Weight = typename InputIt::value_type::value_type;
  union_find uf(v);
  Weight cost = Weight();
  std::set<edge<Weight>> mst;
  while (first != last) {
    const auto& e = *first++;
    if (uf.connected(e.src, e.dst)) continue;
    cost += e.cost;
    mst.insert(e);
    uf.unite(e.src, e.dst);
  }
  return std::make_pair(cost, mst);
}

template <class Tp, class BinaryOperation1, class BinaryOperation2>
class segment_tree {
public:
  using binary_operation1 = BinaryOperation1;
  using binary_operation2 = BinaryOperation2;
  using value_type = Tp;

private:
  std::vector<value_type> c;
  size_t m = 1;
  binary_operation1 op1;  // aggregate
  binary_operation2 op2;  // update

  void init_resize(size_t n) {
    m = 2*n;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    init_resize(n);
    c.assign(m, op1.identity);
  }

  segment_tree(size_t n, const Tp& e,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               BinaryOperation1 op1 = BinaryOperation1(),
               BinaryOperation2 op2 = BinaryOperation2()): op1(op1), op2(op2) {

    static_assert(std::is_convertible<typename ForwardIt::value_type, Tp>::value, "");
    init_resize(std::distance(first, last));
    for (size_t i = m/2; first != last; ++i) c[i] = *first++;
    init_aggregate();
  }

  void update(size_t i, Tp x) {
    i += m/2;
    c[i] = op2(c[i], x);
    while (i > 1) {
      i >>= 1;
      c[i] = op1(c[i<<1], c[i<<1|1]);
    }
  }

  value_type aggregate(size_t l, size_t r) const {
    value_type resl = op1.identity;
    value_type resr = op1.identity;
    l += m/2;
    r += m/2;
    while (l < r) {
      if (l & 1) resl = op1(resl, c[l++]);
      if (r & 1) resr = op1(c[--r], resr);
      size_t j = __builtin_ctzll(l | r);
      l >>= j;
      r >>= j;
    }
    return op1(resl, resr);
  }

  value_type operator [](size_t k) const { return c[k+m/2]; }
  binary_operation1 get_op1() const { return op1; }
  binary_operation2 get_op2() const { return op2; }
};

template <typename Tp>
struct max {
  using value_type = Tp;
  Tp const identity = std::numeric_limits<Tp>::min();
  Tp operator ()(Tp const& x, Tp const& y) const { return std::max(x, y); }
};

template <typename Tp>
struct nop {
  using value_type = Tp;
  Tp const identity = Tp();
  Tp operator ()(Tp const& x, Tp const&) const { return x; }
};

template <typename Tp, typename Sequence>
class path_decomposed_tree {
public:
  using value_type = Tp;
  using sequence = Sequence;
  using binary_operation1 = typename sequence::binary_operation1;
  using binary_operation2 = typename sequence::binary_operation2;

private:
  std::vector<edge<value_type>> M_light;
  std::vector<sequence> M_heavy;
  std::vector<size_t> M_depth;
  std::vector<std::vector<edge<value_type>>> M_chains;
  std::vector<size_t> M_index_h0, M_index_h1, M_index_l;
  binary_operation1 M_op1;

  size_t M_ascend(size_t v) const {
    if (M_index_l[v]+1 != 0) return M_light[M_index_l[v]].src;
    if (M_index_h0[v]+1 != 0) {
      size_t tmp = M_chains[M_index_h0[v]].back().src;
      if (tmp != v) return tmp;
    }
    return -1;
  }

  size_t M_head(size_t v) const {
    if (M_index_h0[v]+1 != 0) return M_chains[M_index_h0[v]].back().src;
    return v;
  }

  size_t M_lca(size_t u, size_t v) const {
    if (u == v) return u;
    if (M_index_h0[u]+1 == 0) u = M_ascend(u);
    if (M_index_h0[v]+1 == 0) v = M_ascend(v);
    while (M_index_h0[u] != M_index_h0[v]) {
      if (M_depth[M_head(u)] < M_depth[M_head(v)]) {
        v = M_ascend(v);
      } else {
        u = M_ascend(u);
      }
    }
    return (M_depth[u] < M_depth[v])? u : v;
  }

  value_type M_step(size_t& u, size_t v, bool rev) const {
    if (u == v) return M_op1.identity;
    if (M_index_l[u]+1 != 0) {
      auto const& e = M_light[M_index_l[u]];
      u = e.src;
      return e.cost;
    }

    assert(M_index_h0[u]+1 != 0);
    size_t nu = M_head(u);
    size_t h0 = M_index_h0[u];
    size_t h1 = M_index_h1[u];
    if (nu == M_head(v)) nu = v;
    size_t h2 = M_index_h1[nu];

    value_type tmp;
    if (!rev) {
      tmp = M_heavy[h0<<1].aggregate(h1, h2);
    } else {
      size_t hlen = M_chains[h0].size();
      tmp = M_heavy[h0<<1|1].aggregate(hlen-h2, hlen-h1);
    }
    u = nu;
    return tmp;
  }

public:
  path_decomposed_tree(graph<value_type> const& g, size_t r = 0) {
    assert(g.is_undirected_tree());

    size_t n = g.size();
    std::vector<size_t> subsize(n, 1);
    std::vector<bool> visited(n);
    M_depth.assign(n, -1);
    M_index_h0.assign(n, -1);
    M_index_h1.assign(n, -1);
    M_index_l.assign(n, -1);

    {
      std::stack<size_t> st;
      st.push(r);
      M_depth[r] = 0;
      while (!st.empty()) {
        size_t v = st.top();
        if (!visited[v]) {
          visited[v] = true;
          for (auto const& e: g[v]) {
            if (visited[e.dst]) continue;
            st.push(e.dst);
            M_depth[e.dst] = M_depth[e.src] + 1;
          }
          continue;
        }

        st.pop();
        size_t max = 0;
        size_t ei = -1;
        for (size_t i = 0; i < g[v].size(); ++i) {
          auto const& e = g[v][i];
          if (M_depth[e.src] > M_depth[e.dst]) continue;
          subsize[e.src] += subsize[e.dst];
          if (max < subsize[e.dst]) {
            if (ei+1 != 0) {
              auto const& e0 = g[v][ei];
              M_index_l[e0.dst] = M_light.size();
              M_light.push_back(e0);
            }
            max = subsize[e.dst];
            ei = i;
          } else {
            M_index_l[e.dst] = M_light.size();
            M_light.push_back(e);
          }
        }
        if (ei+1 == 0) continue;

        auto const& e = g[v][ei];
        size_t ci = M_index_h0[e.dst];

        if (ci+1 == 0) {
          ci = M_chains.size();
          M_chains.emplace_back();
          M_chains[ci].push_back(e);
          M_index_h0[e.src] = M_index_h0[e.dst] = ci;
          M_index_h1[e.src] = 1;
          M_index_h1[e.dst] = 0;
        } else {
          M_chains[ci].push_back(e);
          M_index_h0[e.src] = ci;
          M_index_h1[e.src] = M_chains[ci].size();
        }
      }
    }
    for (auto const& chain: M_chains) {
      std::vector<Tp> xs;
      for (auto const& e: chain)
        xs.push_back(e.cost);

      M_heavy.emplace_back(xs.begin(), xs.end());
      M_heavy.emplace_back(xs.rbegin(), xs.rend());
    }
  }

  value_type aggregate(size_t u, size_t v) const {
    size_t w = M_lca(u, v);
    value_type lhs = M_op1.identity;
    while (u != w) lhs = M_op1(lhs, M_step(u, w, false));
    value_type rhs = M_op1.identity;
    while (v != w) rhs = M_op1(M_step(v, w, true), rhs);
    return M_op1(lhs, rhs);
  }
};

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  std::vector<edge<intmax_t>> es;
  for (size_t i = 0; i < m; ++i) {
    size_t u, v;
    intmax_t w;
    scanf("%zu %zu %jd", &u, &v, &w);
    --u;
    --v;
    es.emplace_back(u, v, w);
  }

  intmax_t mst_cost;
  std::set<edge<intmax_t>> mst_es;
  {
    std::vector<edge<intmax_t>> es0 = es;
    std::sort(es0.begin(), es0.end());
    std::tie(mst_cost, mst_es) = min_spanning_tree(n, es0.begin(), es0.end());
  }

  graph<intmax_t> g(n);
  for (auto const& e: mst_es) g.connect_with(e.src, e.dst, e.cost);

  path_decomposed_tree<intmax_t, segment_tree<intmax_t, max<intmax_t>, nop<intmax_t>>> g0(g);
  for (auto const& e: es) {
    if (mst_es.count(e)) {
      printf("%jd\n", mst_cost);
    } else {
      intmax_t dw = g0.aggregate(e.src, e.dst);
      printf("%jd\n", mst_cost-dw+e.cost);
    }
  }
}
