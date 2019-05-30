template <typename Tp>
class interval_set {
public:
  using value_type = Tp;

private:
  std::set<std::pair<Tp, Tp>> ranges;

public:
  void insert(Tp x) { Tp y = x; insert(x, ++y); }
  void erase(Tp x) { Tp y = x; erase(x, ++y); }

  void insert(Tp lb, Tp ub) {
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

  void erase(Tp lb, Tp ub) {
    if (ranges.empty()) return;

    auto it = ranges.upper_bound({lb, lb});

    if (it != ranges.begin() && !(std::prev(it)->second < lb)) {
      if (!(std::prev(it)->second < ub)) {
        // [ ...* [ ...+ ) ...* )
        --it;
        Tp lb0 = it->first;
        Tp ub0 = it->second;
        ranges.erase(it);
        if (lb0 < lb) ranges.emplace(lb0, lb);
        if (ub < ub0) ranges.emplace(ub, ub0);
        return;
      }

      // [ ...+ )      [ ...+ )*
      //      [ ...+ ) <- erase this
      Tp lb0 = std::prev(it)->first;
      ranges.erase(std::prev(it));
      ranges.emplace(lb0, lb);
    }

    while (it != ranges.end() && !(ub < it->first)) {
      if (ub < it->second) {
        Tp ub0 = it->second;
        ranges.erase(it);
        ranges.emplace(ub, ub0);
        break;
      }
      it = ranges.erase(it);
    }
  }

  size_t num_ranges() const { return ranges.size(); }

  std::pair<Tp, Tp> suprange(Tp x) const {
    if (ranges.empty()) return {x, x};
    auto it = ranges.upper_bound({x, x});

    if (it != ranges.end())
      if (!(x < it->first) && x < it->second) return *it;

    if (it == ranges.begin() || !(x < (--it)->second)) return {x, x};
    return *it;
  }

  bool contains(Tp x) const { return (suprange(x).second != x); }
};
