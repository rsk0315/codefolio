template <class Tp, class Compare = std::less<Tp>>
class sparse_table {
public:
  using value_type = Tp;
  using value_compare = Compare;

private:
  std::vector<std::vector<value_type>> c;
  value_compare comp;

public:
  template <class ForwardIt>
  sparse_table(ForwardIt first, ForwardIt last, Compare comp = std::less<Tp>()):
    c(1, std::vector<value_type>(first, last)), comp(comp)
  {
    size_t n = c[0].size();
    for (size_t i = 1, ii = 1; c.back().size() > ii; (++i, ii <<= 1)) {
      c.emplace_back();
      c.back().reserve(n - ii);
      for (size_t j = ii; j < c[i-1].size(); ++j)
        c[i].push_back(std::min(c[i-1][j], c[i-1][j-ii], comp));
    }
  }

  value_type min(size_t l, size_t r) const {
    assert(l < r);
    size_t e = 63 - __builtin_clzll(r-l);
    r -= (static_cast<size_t>(1) << e) - 1;
    return std::min(c[e][l], c[e][r-1], comp);
  }
};
