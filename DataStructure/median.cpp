template <typename Tp, typename Comparator = std::less<Tp>>
class median {
public:
  using value_type = Tp;
  using key_compare = Comparator;

private:
  using container = std::multiset<value_type, key_compare>;
  using iterator = typename container::iterator;
  container M_c;
  iterator M_it;
  key_compare M_comp = Comparator();
  size_t M_size = 0;
  value_type M_lower_sum = Tp();
  value_type M_upper_sum = Tp();

public:
  median() = default;

  bool empty() const { return M_c.empty(); }
  value_type get() const { return *M_it; }
  value_type lower_sum() const { return M_lower_sum; }
  value_type upper_sum() const { return M_upper_sum; }
  void insert(value_type const& x) {
    ++M_size;
    if (empty()) {
      M_it = M_c.insert(x);
      M_lower_sum += x;
      return;
    }
    M_c.insert(x);
    if (M_comp(x, *M_it)) {
      M_lower_sum += x;
      if (M_size % 2 == 0) {
        M_lower_sum -= *M_it;
        M_upper_sum += *M_it;
        --M_it;
      }
    } else {
      M_upper_sum += x;
      if (M_size % 2 != 0) {
        ++M_it;
        M_lower_sum += *M_it;
        M_upper_sum -= *M_it;
      }
    }
  }

  void erase(value_type const& x) {
    auto it = M_c.find(x);
    if (it == M_c.end()) return;
    if (M_comp(x, *M_it)) {
      M_lower_sum -= x;
      if (M_size % 2 == 0) {
        M_lower_sum += *M_it;
        M_upper_sum -= *M_it;
        --M_it;
      }
    } else {
      M_upper_sum -= x;
      if (M_size % 2 != 0) {
        M_lower_sum -= *M_it;
        M_upper_sum += *M_it;
        --M_it;
      }
    }
    if (M_it == it) {
      M_it = M_c.erase(it, std::next(it));
    } else {
      M_c.erase(it, std::next(it));
    }
    --M_size;
  }
};
