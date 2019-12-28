template <typename Tp>
class integral_intervals {
public:
  using size_type = size_t;
  using value_type = Tp;
  using range_type = std::pair<value_type, value_type>;

private:
  std::set<range_type> ranges;
  size_type M_size = 0;

public:
  integral_intervals() = default;
  integral_intervals(integral_intervals const&) = default;
  integral_intervals(integral_intervals&&) = default;

  integral_intervals& operator =(integral_intervals const&) = default;
  integral_intervals& operator =(integral_intervals&&) = default;

  template <typename InputIt>
  integral_intervals(InputIt first, InputIt last) { assign(first, last); }
  integral_intervals(std::initializer_list<range_type> il) { assign(il.begin(), il.end()); }

  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    while (first != last) {
      insert(first->first, first->second);
      ++first;
    }
  }

  void insert(value_type x) { value_type y = x; insert(x, ++y); }
  void erase(value_type x) { value_type y = x; erase(x, ++y); }

  void insert(value_type lb, value_type ub) {
    if (ranges.empty()) {
      M_size += ub - lb;
      ranges.emplace(lb, ub);
      return;
    }

    auto it = ranges.upper_bound({lb, lb});
    if (it != ranges.begin() && !(std::prev(it)->second < lb)) {
      auto pit = std::prev(it);
      if (!(pit->second < ub)) return;
      lb = pit->first;
      M_size -= pit->second - pit->first;
      ranges.erase(pit);
    }
    while (it != ranges.end() && !(ub < it->first)) {
      if (ub < it->second) ub = it->second;
      M_size -= it->second - it->first;
      it = ranges.erase(it);
    }
    M_size += ub - lb;
    ranges.emplace(lb, ub);
  }

  void erase(value_type lb, value_type ub) {
    if (ranges.empty()) return;

    auto it = ranges.upper_bound({lb, lb});
    if (it != ranges.begin() && !(std::prev(it)->second < lb)) {
      auto pit = std::prev(it);
      if (!(pit->second < ub)) {
        // [ ...* [ ...+ ) ...* )
        --it;
        value_type lb0 = it->first;
        value_type ub0 = it->second;
        M_size -= it->second - it->first;
        ranges.erase(it);
        if (lb0 < lb) {
          M_size += lb - lb0;
          ranges.emplace(lb0, lb);
        }
        if (ub < ub0) {
          M_size += ub0 - ub;
          ranges.emplace(ub, ub0);
        }
        return;
      }

      // [ ...+ )      [ ...+ )*
      //      [ ...+ ) <- erase this
      value_type lb0 = pit->first;
      M_size -= pit->second - pit->first;
      M_size += lb - lb0;
      ranges.erase(pit);
      ranges.emplace(lb0, lb);
    }

    while (it != ranges.end() && !(ub < it->first)) {
      if (ub < it->second) {
        value_type ub0 = it->second;
        M_size -= it->second - it->first;
        M_size += ub0 - ub;
        ranges.erase(it);
        ranges.emplace(ub, ub0);
        break;
      }
      M_size -= it->second - it->first;
      it = ranges.erase(it);
    }
  }

  size_type num_ranges() const { return ranges.size(); }

  range_type suprange(value_type x) const {
    if (ranges.empty()) return {x, x};
    auto it = ranges.upper_bound({x, x});

    if (it != ranges.end())
      if (!(x < it->first) && x < it->second) return *it;

    if (it == ranges.begin() || !(x < (--it)->second)) return {x, x};
    return *it;
  }

  bool contains(value_type x) const { return (suprange(x).second != x); }
  value_type mex() const { return suprange(0).second; }

  bool empty() const noexcept { return (M_size == 0); }
  size_type size() const { return M_size; }
};
