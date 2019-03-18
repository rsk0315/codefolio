#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <functional>
#include <stack>

template <class Tp,
          class BinaryOperation1 = std::plus<Tp>,
          class BinaryOperation2 = std::plus<Tp>>
class segment_tree {
public:
  using binary_operation1 = BinaryOperation1;

private:
  std::vector<Tp> c;
  size_t m = 1;
  BinaryOperation1 op1;  // aggregate
  BinaryOperation2 op2;  // update
  Tp e1;  // unit elements

  void init_resize(size_t n) {
    while (m <= n) m <<= 1;
    c.resize(m);
  }

  void init_aggregate() {
    for (size_t i = m/2; i-- > 1;)
      c[i] = op1(c[i<<1], c[i<<1|1]);
  }

public:
  segment_tree(size_t n,
               const BinaryOperation1& op1, const Tp& e1,
               const BinaryOperation2& op2, Tp e=Tp()): e1(e1) {

    init_resize(n);
    for (size_t i = m/2; i < m; ++i) c[i] = e;
    init_aggregate();
  }

  template <class ForwardIt>
  segment_tree(ForwardIt first, ForwardIt last,
               const BinaryOperation1& op1, const Tp& e1,
               const BinaryOperation2& op2): e1(e1) {

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

  Tp aggregate(size_t l, size_t r) const {
    Tp resl = e1;
    Tp resr = e1;
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
  size_t src, dst;
  Weight cost;

  edge(size_t src, size_t dst, Weight cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const edge<Weight> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
struct graph: public std::vector<std::vector<edge<Weight>>> {
  graph(size_t n): std::vector<std::vector<edge<Weight>>>(n) {}

  void connect_with(size_t src, size_t dst, Weight cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
    (*this)[dst].emplace_back(dst, src, cost);
  }
};

template <class Tp, class Sequence>
class decomposed_graph {
  using binary_operation1 = typename Sequence::binary_operation1;
  std::vector<edge<Tp>> light;
  std::vector<Sequence> heavy;
  std::vector<size_t> depth;
  std::vector<std::vector<edge<Tp>>> chains;
  std::vector<size_t> index_h0, index_h1, index_l;
  binary_operation1 op1;
  Tp e1;

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

  Tp step(size_t& u, size_t v, int rev) const {
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

    Tp tmp;
    if (!rev) {
      tmp = heavy[h0<<1].aggregate(h1, h2);
      // fprintf(stderr, "%zu:%zu(%zu:%zu): %d\n", u, v, h1, h2, tmp);
    } else {
      size_t hlen = chains[h0].size();
      tmp = heavy[h0<<1|1].aggregate(hlen-h2, hlen-h1);
      // fprintf(stderr, "%zu:%zu(%zu:%zu): %d\n", u, v, hlen-h2, hlen-h1, tmp);
    }
    u = nu;
    return tmp;
  }

public:
  template <class BinaryOperation1, class BinaryOperation2>
  decomposed_graph(const graph<Tp>& g,
                   const BinaryOperation1& op1, const Tp& e1,
                   const BinaryOperation2& op2,
                   size_t r=0): op1(op1), e1(e1) {

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

    for (const auto& c: chains)
      for (size_t i = c.size(); i--;)
        fprintf(stderr, "(%zu %zu)%c", c[i].src, c[i].dst,
                i? ' ':'\n');

    for (size_t i = 0; i < n; ++i)
      if (index_h0[i]+1 != 0)
        fprintf(stderr, "%zu in chain #%zu:%zu\n", i, index_h0[i], index_h1[i]);

    for (size_t i = 0; i < n; ++i)
      fprintf(stderr, "climb(%zu): %zd\n", i, climb(i));

    // for (size_t i = 0; i < n; ++i)
    //   for (size_t j = 0; j < n; ++j)
    //     fprintf(stderr, "lca(%zu, %zu): %zu\n", i, j, lca(i, j));

    for (const auto& chain: chains) {
      std::vector<Tp> xs;
      for (const auto& e: chain)
        xs.push_back(e.cost);

      heavy.emplace_back(xs.begin(), xs.end(), op1, e1, op2);
      heavy.emplace_back(xs.rbegin(), xs.rend(), op1, e1, op2);
    }

    // for (size_t i = 0; i < n; ++i)
    //   for (size_t j = 0; j < n; ++j)
    //     fprintf(stderr, "%zu--%zu: %d\n", i, j, aggregate(i, j));
  }

  Tp aggregate(size_t u, size_t v) const {
    size_t w = lca(u, v);
    Tp res = e1;
    while (u != w) res = op1(res, step(u, w, 0));
    {
      Tp tmp = e1;
      while (v != w) tmp = op1(step(v, w, 1), tmp);
      res = op1(res, tmp);
    }
    return res;
  }
};

int main() {
  size_t n;
  scanf("%zu", &n);

  graph<int> g(n);
  std::vector<std::vector<int>> gg(n, std::vector<int>(n, 1e9));
  for (size_t i = 1; i < n; ++i) {
    size_t s, d;
    int w;
    scanf("%zu %zu %d", &s, &d, &w);
    // --s;
    // --d;
    g.connect_with(s, d, w);
    gg[s][d] = gg[d][s] = w;
  }

  for (size_t i = 0; i < n; ++i) gg[i][i] = 0;

  for (size_t k = 0; k < n; ++k)
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
        gg[i][j] = std::min(gg[i][j], gg[i][k]+gg[k][j]);

  // struct { bool operator ()(int x, int y) const { return std::min(x, y); } } min;
  // const int inf = 1e9;
  // decomposed_graph<int, segment_tree<int, decltype(min), decltype(min)>> dg(g, min, inf, min);
  decomposed_graph<int, segment_tree<int, std::plus<int>, std::plus<int>>> dg(g, std::plus<int>(), 0, std::plus<int>());

    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j) {
        int d0 = dg.aggregate(i, j);
        int d1 = gg[i][j];
        fprintf(stderr, "[%zu][%zu]: %d? (%d)\n", i, j, d0, d1);
        assert(d0 == d1);
      }
}
