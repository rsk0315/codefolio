template <typename ForwardIt, class BinaryPredicate = std::equal_to<>>
class knuth_morris_pratt_searcher {
public:
  using size_type = size_t;
  using value_type = typename ForwardIt::value_type;

private:
  static constexpr size_type S_npos = -1;
  std::vector<value_type> M_pat;
  std::vector<size_type> M_fail;
  BinaryPredicate M_pred;

public:
  knuth_morris_pratt_searcher(ForwardIt pat_first, ForwardIt pat_last,
                              BinaryPredicate pred = BinaryPredicate()):
    M_pat(pat_first, pat_last), M_fail(M_pat.size()+1), M_pred(pred)
  {
    M_fail[0] = S_npos;
    for (size_type i = 0, j = S_npos; i < M_pat.size();) {
      while (j != S_npos && !pred(M_pat[i], M_pat[j])) j = M_fail[j];
      ++i, ++j;
      M_fail[i] = (pred(i < M_pat.size() && M_pat[i], M_pat[j])? M_fail[j]: j);
    }
  }

  template <typename ForwardIt2>
  std::vector<std::pair<ForwardIt2, ForwardIt2>> search_all(
      ForwardIt2 first, ForwardIt2 last
  ) const {
    std::vector<std::pair<ForwardIt2, ForwardIt2>> res;
    std::deque<ForwardIt2> start;
    size_type j = 0;
    for (auto it = first; it != last;) {
      start.push_back(it);
      while (j != S_npos && !M_pred(M_pat[j], *it)) j = M_fail[j];
      ++it;
      if (start.size() > M_pat.size()) start.pop_front();
      if (++j == M_pat.size()) {
        fprintf(stderr, "pushed: [%td:%td]\n", start.front()-first, it-first);
        res.emplace_back(start.front(), it);
        j = M_fail[j];
      }
    }
    return res;
  }

  template <typename ForwardIt2>
  std::pair<ForwardIt2, ForwardIt2> search_first(
      ForwardIt2 first, ForwardIt2 last
  ) const {
    size_type j = 0;
    for (auto it = first; it != last;) {
      while (j != S_npos && !M_pred(M_pat[j], *it)) j = M_fail[j];
      ++it;
      if (++j == M_pat.size()) return {std::prev(it, j), it};
    }
    return {last, last};
  }

  template <typename ForwardIt2>
  std::pair<ForwardIt2, ForwardIt2> operator ()(
      ForwardIt2 first, ForwardIt2 last
  ) const { return search_first(first, last); }
};
