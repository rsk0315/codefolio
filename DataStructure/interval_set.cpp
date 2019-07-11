template <typename Tp>
class interval_set {
public:
  using size_type = size_t;
  using value_type = Tp;
  using range_type = std::pair<value_type, value_type>;

private:
  std::set<range_type> ranges;

public:
  void insert(value_type x) { value_type y = x; insert(x, ++y); }
  void erase(value_type x) { value_type y = x; erase(x, ++y); }

  void insert(value_type lb, value_type ub) {
    if (ranges.empty()) {
      ranges.emplace(lb, ub);
      return;
    }

    auto it = ranges.upper_bound({lb, lb});
    if (it != ranges.begin() && !(std::prev(it)->second < lb)) {
      if (!(std::prev(it)->second < ub)) return;
      lb = std::prev(it)->first;
      ranges.erase(std::prev(it));
    }
    while (it != ranges.end() && !(ub < it->first)) {
      if (ub < it->second) ub = it->second;
      it = ranges.erase(it);
    }
    ranges.emplace(lb, ub);
  }

  void erase(value_type lb, value_type ub) {
    if (ranges.empty()) return;

    auto it = ranges.upper_bound({lb, lb});

    if (it != ranges.begin() && !(std::prev(it)->second < lb)) {
      if (!(std::prev(it)->second < ub)) {
        // [ ...* [ ...+ ) ...* )
        --it;
        value_type lb0 = it->first;
        value_type ub0 = it->second;
        ranges.erase(it);
        if (lb0 < lb) ranges.emplace(lb0, lb);
        if (ub < ub0) ranges.emplace(ub, ub0);
        return;
      }

      // [ ...+ )      [ ...+ )*
      //      [ ...+ ) <- erase this
      value_type lb0 = std::prev(it)->first;
      ranges.erase(std::prev(it));
      ranges.emplace(lb0, lb);
    }

    while (it != ranges.end() && !(ub < it->first)) {
      if (ub < it->second) {
        value_type ub0 = it->second;
        ranges.erase(it);
        ranges.emplace(ub, ub0);
        break;
      }
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
};
