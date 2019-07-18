template <typename Tp>
class linear_minima {
public:
  using size_type = size_t;
  using value_type = Tp;

private:
  using interval_type = std::pair<value_type, value_type>;
  using line_type = std::pair<value_type, value_type>;
  std::map<interval_type, line_type> M_lines;
  std::map<line_type, interval_type, std::greater<line_type>> M_intervals;
  static value_type const S_min = std::numeric_limits<value_type>::min();
  static value_type const S_max = std::numeric_limits<value_type>::max();

  static value_type S_floor(value_type x, value_type y) {
    value_type q = x / y;
    value_type r = x % y;
    if (r < 0) --q;
    return q;
  }

  static value_type S_ceil(value_type x, value_type y) {
    value_type q = x / y;
    value_type r = x % y;
    if (r > 0) ++q;
    return q;
  }

public:
  linear_minima() = default;
  linear_minima(linear_minima const&) = default;
  linear_minima(linear_minima&&) = default;
  linear_minima& operator =(linear_minima const&) = default;
  linear_minima& operator =(linear_minima&&) = default;

  bool push(value_type const& a, value_type const& b) {
    // try to push y = ax+b; return true if it is actually pushed
    if (M_lines.empty()) {
      M_lines[interval_type(S_min, S_max)] = line_type(a, b);
      M_intervals[line_type(a, b)] = interval_type(S_min, S_max);
      return true;
    }

    auto it = M_intervals.lower_bound(line_type(a, S_min));
    value_type lb = S_min;
    value_type ub = S_max;

    auto it1 = it;  // next one
    if (it != M_intervals.end() && it->first.first == a) {
      if (it->first.second <= b) return false;
      M_lines.erase(it->second);
      it1 = M_intervals.erase(it);
    }

    auto it0 = M_intervals.end();  // previous one
    if (it != M_intervals.begin()) it0 = std::prev(it);

    if (it0 != M_intervals.end()) {
      value_type a0, b0;
      std::tie(a0, b0) = it0->first;
      lb = S_ceil(b-b0, -(a-a0));  // XXX this may cause overflow
    }
    if (it1 != M_intervals.end()) {
      value_type a1, b1;
      std::tie(a1, b1) = it1->first;
      ub = S_floor(b1-b, -(a1-a));  // XXX this may cause overflow
    }
    if (ub < lb) return false;

    if (it0 != M_intervals.end()) {
      while (lb <= it0->second.first) {
        M_lines.erase(it0->second);
        it0 = M_intervals.erase(it0);
        if (it0 == M_intervals.begin()) {
          it0 = M_intervals.end();
          break;
        }
        --it0;
        value_type a0, b0;
        std::tie(a0, b0) = it0->first;
        lb = S_ceil(b-b0, -(a-a0));
      }
    }

    while (it1 != M_intervals.end() && it1->second.second <= ub) {
      M_lines.erase(it1->second);
      it1 = M_intervals.erase(it1);
      value_type a1, b1;
      std::tie(a1, b1) = it1->first;
      ub = S_floor(b1-b, -(a1-a));
    }

    if (it0 != M_intervals.end()) {
      value_type a0, b0, l0, u0;
      std::tie(a0, b0) = it0->first;
      std::tie(l0, u0) = it0->second;
      it0->second.second = std::min(u0, lb-1);
      M_lines.erase(interval_type(l0, u0));
      M_lines[it0->second] = it0->first;
    }
    if (it1 != M_intervals.end()) {
      value_type a1, b1, l1, u1;
      std::tie(a1, b1) = it1->first;
      std::tie(l1, u1) = it1->second;
      it1->second.first = std::max(l1, ub+1);
      M_lines.erase(interval_type(l1, u1));
      M_lines[it1->second] = it1->first;
    }

    M_lines[interval_type(lb, ub)] = line_type(a, b);
    M_intervals[line_type(a, b)] = interval_type(lb, ub);

    return true;
  }

  value_type at(value_type const& x) {
    // return the minimum value at x
    value_type a, b;
    std::tie(a, b) = (--M_lines.upper_bound(interval_type(x, S_max)))->second;
    return a*x + b;
  }
};

template <typename Tp> Tp const linear_minima<Tp>::S_min;
template <typename Tp> Tp const linear_minima<Tp>::S_max;
