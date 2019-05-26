#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <functional>
#include <stack>
#include <tuple>
#include <set>

template <class Tp,
          class BinaryOperation1 = std::plus<Tp>,
          class BinaryOperation2 = std::plus<Tp>>
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
  value_type e1;  // unit elements

  void init_resize(size_t n) {
    while (m < n+n) m <<= 1;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n, const value_type& e,
               const value_type& e1): e1(e1) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               const value_type& e1): e1(e1) {

    static_assert(std::is_same<Tp, typename ForwardIt::value_type>::value, "");
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
    value_type resl = e1;
    value_type resr = e1;
    l += m/2;
    r += m/2;
    while (l < r) {
      if (l & 1) resl = op1(resl, c[l++]);
      if (r & 1) resr = op1(c[--r], resr);
      l >>= 1;
      r >>= 1;
    }
    return op1(resl, resr);
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
};

template <class Tp, class Sequence>
class decomposed_graph {
public:
  using value_type = Tp;
  using sequence_type = Sequence;
  using binary_operation1 = typename sequence_type::binary_operation1;
  using binary_operation2 = typename sequence_type::binary_operation2;

private:
  std::vector<edge<value_type>> light;
  std::vector<sequence_type> heavy;
  std::vector<size_t> depth;
  std::vector<std::vector<edge<value_type>>> chains;
  std::vector<size_t> index_h0, index_h1, index_l;
  binary_operation1 op1;
  value_type e1;

  size_t climb(size_t v) const {
    if (~index_l[v]) return light[index_l[v]].src;
    if (~index_h0[v]) {
      size_t tmp = chains[index_h0[v]].back().src;
      if (tmp != v) return tmp;
    }
    return -1;
  }

  size_t head(size_t v) const {
    if (~index_h0[v]) return chains[index_h0[v]].back().src;
    return v;
  }

  size_t lca(size_t u, size_t v) const {
    if (u == v) return u;
    if (index_h0[u]+1 == 0) u = climb(u);
    if (index_h0[v]+1 == 0) v = climb(v);
    while (index_h0[u] != index_h0[v]) {
      if (depth[head(u)] < depth[head(v)]) {
        v = climb(v);
      } else {
        u = climb(u);
      }
    }
    return (depth[u] < depth[v])? u : v;
  }

  value_type step(size_t& u, size_t v, int rev) const {
    if (u == v) return e1;
    if (~index_l[u]) {
      const auto& e = light[index_l[u]];
      u = e.src;
      return e.cost;
    }

    assert(~index_h0[u]);
    size_t nu = head(u);
    size_t h0 = index_h0[u];
    size_t h1 = index_h1[u];
    if (nu == head(v)) nu = v;
    size_t h2 = index_h1[nu];

    fprintf(stderr, "nu: %zu, h0: %zu, h1: %zu, h2: %zu\n", nu, h0, h1, h2);
    value_type tmp;
    if (!rev) {
      fprintf(stderr, "!rev: [%zu, %zu)\n", h1, h2);
      tmp = heavy[h0<<1].aggregate(h1, h2);
    } else {
      size_t hlen = chains[h0].size();
      fprintf(stderr, "rev: [%zu, %zu)\n", hlen-h2, hlen-h1);
      tmp = heavy[h0<<1|1].aggregate(hlen-h2, hlen-h1);
    }
    u = nu;
    return tmp;
  }

public:
  decomposed_graph(const graph<value_type>& g,
                   const value_type& e1, size_t r=0): e1(e1) {

    size_t n = g.size();
    std::vector<size_t> subsize(n, 1);
    std::vector<bool> visited(n);
    depth.assign(n, -1);
    index_h0.assign(n, -1);
    index_h1.assign(n, -1);
    index_l.assign(n, -1);

    {
      std::stack<size_t> st;
      st.push(r);
      depth[r] = 0;
      while (!st.empty()) {
        size_t v = st.top();
        if (!visited[v]) {
          visited[v] = true;
          for (const auto& e: g[v]) {
            if (visited[e.dst]) continue;
            st.push(e.dst);
            depth[e.dst] = depth[e.src] + 1;
          }
          continue;
        }

        st.pop();
        size_t max = 0;
        size_t ei = -1;
        for (size_t i = 0; i < g[v].size(); ++i) {
          const auto& e = g[v][i];
          if (depth[e.src] > depth[e.dst]) continue;
          subsize[e.src] += subsize[e.dst];
          if (max < subsize[e.dst]) {
            if (ei+1 != 0) {
              const auto& e0 = g[v][ei];
              index_l[e0.dst] = light.size();
              light.push_back(e0);
            }
            max = subsize[e.dst];
            ei = i;
          } else {
            index_l[e.dst] = light.size();
            light.push_back(e);
          }
        }
        if (ei+1 == 0) continue;

        const auto& e = g[v][ei];
        size_t ci = index_h0[e.dst];

        if (ci+1 == 0) {
          ci = chains.size();
          chains.emplace_back();
          chains[ci].push_back(e);
          index_h0[e.src] = index_h0[e.dst] = ci;
          index_h1[e.src] = 1;
          index_h1[e.dst] = 0;
        } else {
          chains[ci].push_back(e);
          index_h0[e.src] = ci;
          index_h1[e.src] = chains[ci].size();
        }
      }
    }

    for (const auto& chain: chains) {
      std::vector<Tp> xs;
      for (const auto& e: chain)
        xs.push_back(e.cost);

      for (size_t i = 0; i < xs.size(); ++i)
        fprintf(stderr, "%jd%c", xs[i], i+1<xs.size()? ' ':'\n');

      heavy.emplace_back(xs.begin(), xs.end(), e1);
      heavy.emplace_back(xs.rbegin(), xs.rend(), e1);
    }
  }

  value_type aggregate(size_t u, size_t v) const {
    size_t w = lca(u, v);

    value_type lhs = e1;
    while (u != w) lhs = op1(lhs, step(u, w, 0));
    value_type rhs = e1;
    while (v != w) rhs = op1(step(v, w, 1), rhs);

    return op1(lhs, rhs);
  }
};

class union_find {
  std::vector<intmax_t> tree;

public:
  union_find(size_t n): tree(n, -1) {}

  size_t find(size_t v) {
    if (tree[v] < 0)
      return v;

    return (tree[v] = find(tree[v]));
  }

  bool unite(size_t u, size_t v) {
    u = find(u);
    v = find(v);
    if (u == v)
      return false;

    if (-tree[u] >= -tree[v])
      std::swap(u, v);

    tree[v] += tree[u];
    tree[u] = v;
    return true;
  }

  bool connected(size_t u, size_t v) {
    return (find(u) == find(v));
  }

  size_t size(size_t v) {
    return -tree[find(v)];
  }
};

template <class ForwardIt>
auto min_spanning_tree(size_t v, ForwardIt first, ForwardIt last) {
  using Weight = typename ForwardIt::value_type::value_type;
  // return type: std::pair<Weight, std::set<edge<Weight>>

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

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  graph<intmax_t> g(n);
  std::vector<edge<intmax_t>> es0;
  for (size_t i = 0; i < m; ++i) {
    size_t u, v;
    intmax_t w;
    scanf("%zu %zu %jd", &u, &v, &w);
    --u;
    --v;
    g.connect_with(u, v, w);
    es0.emplace_back(u, v, w);
  }

  auto es = es0;
  std::sort(es.begin(), es.end());

  intmax_t cost;
  std::set<edge<intmax_t>> mst;
  std::tie(cost, mst) = min_spanning_tree(n, es.begin(), es.end());

  graph<intmax_t> g0(n);
  for (const auto& e: mst)
    g0.connect_with(e.src, e.dst, e.cost);

  struct {
    intmax_t operator ()(intmax_t x, intmax_t y) const {
      return std::max(x, y);
    }
  } max;
  using Sequence = segment_tree<intmax_t, decltype(max)>;
  decomposed_graph<intmax_t, Sequence> dg(g0, 0);

  for (const auto& e: es0) {
    size_t s = e.src;
    size_t d = e.dst;
    if (mst.count(e) || mst.count({d, s})) {
      printf("%jd\n", cost);
      continue;
    }

    printf("%jd\n", cost + e.cost - dg.aggregate(s, d));
  }
}
