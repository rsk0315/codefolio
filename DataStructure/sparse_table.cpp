template <class Tp, class Compare = std::less<Tp>>
class sparse_table {
public:
  using size_type = size_t;
  using value_type = Tp;
  using value_compare = Compare;

private:
  std::vector<std::vector<value_type>> c;
  value_compare comp;

public:
  sparse_table() = default;
  sparse_table(sparse_table const&) = default;
  sparse_table(sparse_table&&) = default;

  template <class InputIt>
  sparse_table(InputIt first, InputIt last,
               value_compare comp = value_compare()):
    c(1, std::vector<value_type>(first, last)), comp(comp)
  {
    size_type n = c[0].size();
    for (size_type i = 1, ii = 1; c.back().size() > ii; (++i, ii <<= 1)) {
      c.emplace_back();
      c.back().reserve(n - ii);
      for (size_type j = ii; j < c[i-1].size(); ++j)
        c[i].push_back(std::min(c[i-1][j], c[i-1][j-ii], comp));
    }
  }

  sparse_table& operator =(sparse_table const&) = default;
  sparse_table& operator =(sparse_table&&) = default;

  value_type get(size_type l, size_type r) const {
    assert(l < r);
    size_type e = 63 - __builtin_clzll(r-l);
    r -= (1_zu << e) - 1;
    return std::min(c[e][l], c[e][r-1], comp);
  }
};
