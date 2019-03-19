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

    value_type tmp;
    if (!rev) {
      tmp = heavy[h0<<1].aggregate(h1, h2);
    } else {
      size_t hlen = chains[h0].size();
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
