template <typename WeightType>
class weighted_edge {
public:
  using size_type = size_t;
  using weight_type = WeightType;

protected:
  size_type M_src, M_dst;
  weight_type M_weight;

public:
  weighted_edge() = default;
  weighted_edge(weighted_edge const&) = default;
  weighted_edge(weighted_edge&&) = default;

  weighted_edge(size_type s, size_type d, weight_type w):
    M_src(s), M_dst(d), M_weight(w)
  {}

  weighted_edge& operator =(weighted_edge const&) = default;
  weighted_edge& operator =(weighted_edge&&) = default;

  bool operator <(weighted_edge const& other) const {
    if (M_weight < other.M_weight) return true;
    if (other.M_weight < M_weight) return false;
    if (M_src != other.M_src) return M_src < other.M_src;
    return M_dst < other.M_dst;
  }

  size_type source() const { return M_src; }
  size_type target() const { return M_dst; }
  size_type weight() const { return M_weight; }
};

template <typename WeightType>
class colored_weighted_edge: public weighted_edge<WeightType> {
public:
  using size_type = size_t;
  using weight_type = WeightType;
  using color_type = size_type;

protected:
  color_type M_color;

public:
  colored_weighted_edge() = default;
  colored_weighted_edge(colored_weighted_edge const&) = default;
  colored_weighted_edge(colored_weighted_edge&&) = default;

  colored_weighted_edge(size_type s, size_type d, color_type c, weight_type w):
    weighted_edge<WeightType>(s, d, w), M_color(c)
  {}

  colored_weighted_edge& operator =(colored_weighted_edge const&) = default;
  colored_weighted_edge& operator =(colored_weighted_edge&&) = default;

  bool operator <(colored_weighted_edge const& other) const {
    if (this->M_weight < other.M_weight) return true;
    if (other.M_weight < this->M_weight) return false;
    if (M_color != other.M_color) return M_color < other.M_color;
    if (this->M_src != other.M_src) return this->M_src < other.M_src;
    return this->M_dst < other.M_dst;
  }

  size_type color() const { return M_color; }
};

struct directed_tag {};
struct undirected_tag {};

template <typename Edge, typename Tag>
class adjacency_list {};

template <typename Edge>
class adjacency_list<Edge, undirected_tag> {
public:
  using size_type = size_t;
  using edge_type = Edge;
  using weight_type = typename Edge::weight_type;

private:
  std::vector<std::vector<edge_type>> M_g;

public:
  adjacency_list() = default;
  adjacency_list(adjacency_list const&) = default;
  adjacency_list(adjacency_list&&) = default;
  adjacency_list(size_type n): M_g(n) {}

  template <typename... Args>
  void emplace(size_type src, size_type dst, Args... args) {
    M_g[src].emplace_back(src, dst, args...);
    M_g[dst].emplace_back(dst, src, args...);
  }

  size_type size() const { return M_g.size(); }
  std::vector<edge_type> const& operator [](size_type i) const { return M_g[i]; }
};
