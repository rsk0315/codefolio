template <class Tp>
class lines {
  std::map<Tp, Tp> ll;

  bool is_redundant(const typename std::map<Tp, Tp>::iterator& it) const {
    if (it == ll.begin() || it == ll.end()) return false;
    auto next = std::next(it);
    if (next == ll.end()) return false;
    auto prev = std::prev(it);

    Tp a1, b1, a2, b2, a3, b3;
    std::tie(a1, b1) = *prev;
    std::tie(a2, b2) = *it;
    std::tie(a3, b3) = *next;

    fprintf(stderr, "[%d, %d] [%d, %d] [%d, %d]\n", a1, b1, a2, b2, a3, b3);
    return (b1-b2) * (a3-a2) <= (b2-b3) * (a2-a1);
  }

public:
  void append(Tp a, Tp b) {
    if (ll.empty()) {
      ll.emplace(a, b);
      fprintf(stderr, "(+) %dx%+d: empty set\n", a, b);
      return;
    }

    auto it = ll.lower_bound(a);
    if (it != ll.end() && it->first == a) {
      if (it->second <= b) {
        fprintf(stderr, "(x) %dx%+d: one with less constant exists\n", a, b);
        return;
      }
      it->second = b;
      fprintf(stderr, "(+) %dx%+d: update constant\n", a, b);
    } else if (it == ll.begin() || it == ll.end()) {
      it = ll.emplace_hint(it, a, b);
      fprintf(stderr, "(+) %dx%+d: either ends\n", a, b);
    } else {
      it = ll.emplace_hint(it, a, b);
      if (is_redundant(it)) {
        ll.erase(it);
        fprintf(stderr, "(x) %dx%+d: redundant\n", a, b);
        return;
      }
      fprintf(stderr, "(+) %dx%+d: normally\n", a, b);
    }

    auto prev = std::prev(it);
    while (is_redundant(prev)) {
      fprintf(stderr, "(-) %dx%+d: prev\n", prev->first, prev->second);
      ll.erase(prev);
      prev = std::prev(it);
    }

    auto next = std::next(it);
    while (is_redundant(next)) {
      fprintf(stderr, "(-) %dx%+d: next\n", next->first, next->second);
      next = ll.erase(next);
    }
  }

  Tp min_at(Tp x) const {
    assert(!ll.empty());
    if (ll.size() == 1) {
      Tp a = ll.begin()->first;
      Tp b = ll.begin()->second;
      return a*x + b;
    }

    Tp lb = ll.begin()->first;
    auto last = std::prev(ll.end());
    Tp ub = last->first;
    while (ub-lb > 1) {
      Tp mid = (lb+ub) / 2;
      auto it1 = ll.upper_bound(mid);
      auto it0 = std::prev(it1);
      Tp a0, b0, a1, b1;
      std::tie(a0, b0) = *it0;
      std::tie(a1, b1) = *it1;
      ((a0*x+b0 < a1*x+b1)? ub:lb) = mid;
    }

    auto it1 = ll.upper_bound(lb);
    auto it0 = std::prev(it1);

    Tp a0, b0, a1, b1;
    std::tie(a0, b0) = *it0;
    std::tie(a1, b1) = *it1;
    return std::min(a0*x+b0, a1*x+b1);
  }

  void inspect() const {
    for (const auto& p: ll)
      fprintf(stderr, "%dx%+d\n", p.first, p.second);
  }
};
