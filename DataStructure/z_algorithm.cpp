template <typename ForwardIt, class BinaryPredicate = std::equal_to<>>
class z_array {
public:
  using size_type = size_t;
  using value_type = typename ForwardIt::value_type;

private:
  std::vector<value_type> M_pat;
  std::vector<size_type> M_z;
  BinaryPredicate M_pred;

public:
  z_array(z_array const&) = default;
  z_array(z_array&&) = default;

  z_array(ForwardIt pat_first, ForwardIt pat_last,
          BinaryPredicate pred = BinaryPredicate()):
    M_pat(pat_first, pat_last), M_z(M_pat.size()), M_pred(pred)
  {
    if (pat_first == pat_last) return;
    M_z[0] = M_pat.size();
    for (size_type i = 1, j = 0; i < M_pat.size();) {
      while (i+j < M_pat.size() && M_pred(M_pat[j], M_pat[i+j])) ++j;
      M_z[i] = j;
      if (j == 0) {
        ++i;
        continue;
      }
      size_type k = 1;
      while (i+k < M_pat.size() && k+M_z[k] < j) M_z[i+k] = M_z[k], ++k;
      i += k;
      j -= k;
    }
  }

  z_array& operator =(z_array const&) = default;
  z_array& operator =(z_array&&) = default;

  template <typename ForwardIt2>
  std::vector<std::pair<ForwardIt2, ForwardIt2>> search_all(
      ForwardIt2 first, ForwardIt2 last
  ) const {
    if (first == last) return {{last, last}};
    std::vector<std::pair<ForwardIt2, ForwardIt2>> res;
    if (M_pat.empty()) {
      for (auto it = first; it != last; ++it) res.emplace_back(it, it);
      res.emplace_back(last, last);
      return res;
    }

    std::deque<ForwardIt2> start;
    size_type i = 0;
    size_type j = 0;
    for (auto it = first; it != last;) {
      while (it != last && j < M_pat.size() && M_pred(M_pat[j], *it)) {
        ++j;
        start.push_back(it++);
        if (start.size() > M_pat.size()) start.pop_front();
      }
      if (j == 0) {
        ++i;
        start.push_back(it++);
        if (start.size() > M_pat.size()) start.pop_front();
        continue;
      }
      if (j == M_pat.size()) res.emplace_back(start.front(), it);
      size_type k = 1;
      while (k < M_pat.size() && k+M_z[k] < j) ++k;
      i += k;
      j -= k;
    }
    return res;
  }

  template <typename ForwardIt2>
  std::pair<ForwardIt2, ForwardIt2> search_first(
      ForwardIt2 first, ForwardIt2 last
  ) const {
    if (M_pat.empty()) return {first, first};
    if (first == last) return {last, last};

    std::deque<ForwardIt2> start;
    size_type i = 0;
    size_type j = 0;
    for (auto it = first; it != last;) {
      while (it != last && j < M_pat.size() && M_pred(M_pat[j], *it)) {
        ++j;
        start.push_back(it++);
        if (start.size() > M_pat.size()) start.pop_front();
      }
      if (j == 0) {
        ++i;
        start.push_back(it++);
        if (start.size() > M_pat.size()) start.pop_front();
        continue;
      }
      if (j == M_pat.size()) return {start.front(), it};
      size_type k = 1;
      while (k < M_pat.size() && k+M_z[k] < j) ++k;
      i += k;
      j -= k;
    }
    return {last, last};
  }

  template <typename ForwardIt2>
  std::pair<ForwardIt2, ForwardIt2> operator ()(
      ForwardIt2 first, ForwardIt2 last
  ) const { return search_first(first, last); }
};

template <typename ForwardIt, typename BinaryPredicate = std::equal_to<>>
auto make_z_array(ForwardIt first, ForwardIt last,
                  BinaryPredicate pred = BinaryPredicate()) {
  return z_array<ForwardIt, BinaryPredicate>(first, last, pred);
}
