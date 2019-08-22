template <typename RangeQuery>
class heavy_light_decomposed_tree {
public:
  using size_type = size_t;
  using value_type = typename RangeQuery::value_type;
  using range_query = RangeQuery;

private:

public:
  template <typename Tree>
  heavy_light_decomposed_tree(Tree const& tree);

  void modify_vertices(size_type u, size_type v, value_type const& x);
  void modify_edges(size_type u, size_type v, value_type const& x);

  value_type fold_vertices(size_type u, size_type v) const;
  value_type fold_edges(size_type u, size_type v) const;
};
