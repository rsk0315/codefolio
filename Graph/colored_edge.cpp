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
