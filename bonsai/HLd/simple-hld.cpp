#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>

template <typename RangeQuery>
class heavy_light_decomposed_tree {
public:
  using size_type = size_t;
  using range_query_type = RangeQuery;
  using value_type = typename rangequery::value_type;

private:
  std::vector<size_type> M_h_child, M_h_ancestor, M_parent;
  std::vector<size_type> M_depth, M_index;

public:
  heavy_light_decomposed_tree() = default;
  heavy_light_decomposed_tree(heavy_light_decomposed_tree const&) = default;
  heavy_light_decomposed_tree(heavy_light_decomposed_tree&&) = default;

  template <typename Weight>
  heavy_light_decomposed_tree(graph<Weight> const& g, size_t r = 0):
    M_h_child(g.size(), -1), M_h_ancestor(g.size()), M_parent(g.size()),
    M_depth(g.size()), M_index(g.size())
  {
    M_parent[r] = -1;
    M_depth[r] = 0;
    M_dfs(g, r);
    for (size_type i = 0, pos = 0; i < g.size(); ++i) {
      if (!(M_parent[i]+1 == 0 || M_h_child[parent[i]] != i)) continue;
      for (size_type j = i; j+1 != 0; j = M_h_child[j]) {
        M_h_ancestor[j] = i;
        M_index[j] = pos++;
      }
    }
  }

};
