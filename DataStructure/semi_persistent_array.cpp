template <typename Tp>
class semi_persistent_array {
public:
  using size_type = size_t;
  using value_type = Tp;

private:
  std::vector<std::vector<std::pair<size_type, value_type>>> M_c;
  size_type last = 0;

public:
  semi_persistent_array() = default;
  semi_persistent_array(semi_persistent_array const&) = default;
  semi_persistent_array(semi_persistent_array&&) = default;
  semi_persistent_array(size_type n, value_type const& x = value_type{}):
    M_c(n, {std::make_pair(0, x)})
  {}

  void set(size_type i, value_type const& x) {
    M_c[i].emplace_back(++last, x);
  }

  value_type get(size_type i, size_type t = -1) const {
    if (M_c[i].back().first <= t) return M_c[i].back().second;
    size_type lb = 0;
    size_type ub = M_c[i].size();
    while (ub-lb > 1) {
      size_type mid = (lb+ub) >> 1;
      ((M_c[i][mid].first <= t)? lb: ub) = mid;
    }
    return M_c[i][lb].second;
  }
};
