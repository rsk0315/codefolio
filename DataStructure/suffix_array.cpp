template <typename Tp, typename Compare = std::less<Tp>>
class suffix_array {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using compare = Compare;

private:
  std::vector<value_type> M_raw;
  size_type M_size;
  std::vector<size_type> M_rank;
  std::vector<size_type> M_sa;
  compare M_comp;

  void M_build() {
    M_sa.resize(M_size);
    std::iota(M_sa.begin(), M_sa.end(), 0_zu);

    {
      auto comp0 = [&](size_type i, size_type j) -> bool {
        // comparison between value_type's, not size_type's
        if (i < M_raw.size()) {
          if (j >= M_raw.size()) return false;
          if (j < M_raw.size()) {
            if (M_comp(M_raw[i], M_raw[j])) return true;
            if (M_comp(M_raw[j], M_raw[i])) return false;
          }
        } else if (j < M_raw.size()) {
          return true;
        }
        size_type k = 1;
        if (i+k < M_raw.size()) {
          if (j+k < M_raw.size()) return M_comp(M_raw[i+k], M_raw[j+k]);
          return false;
        }
        return j+k < M_raw.size();
      };
      std::sort(M_sa.begin(), M_sa.end(), comp0);
      std::vector<size_type> tmp(M_size);
      tmp[M_sa[0]] = 0_zu;
      for (size_type i = 1; i < M_size; ++i) {
        tmp[M_sa[i]] = tmp[M_sa[i-1]];
        if (comp0(M_sa[i-1], M_sa[i])) ++tmp[M_sa[i]];
      }
      M_rank = std::move(tmp);
    }

    for (size_type k = 2; k < M_size; k <<= 1) {
      auto comp = [&](size_type i, size_type j) -> bool {
        if (M_rank[i] < M_rank[j] || M_rank[j] < M_rank[i])
          return (M_rank[i] < M_rank[j]);
        size_type const& ri = ((i+k < M_size)? M_rank[i+k]+1: 0_zu);
        size_type const& rj = ((j+k < M_size)? M_rank[j+k]+1: 0_zu);
        return ri < rj;
      };
      std::sort(M_sa.begin(), M_sa.end(), comp);
      std::vector<size_type> tmp(M_size);
      tmp[M_sa[0]] = 0;
      for (size_type i = 1; i < M_size; ++i) {
        tmp[M_sa[i]] = tmp[M_sa[i-1]];
        if (comp(M_sa[i-1], M_sa[i])) ++tmp[M_sa[i]];
      }
      M_rank = std::move(tmp);
    }

    // for (size_type i = 0; i < M_sa.size(); ++i)
    //   fprintf(stderr, "%zu%c", M_sa[i], i+1<M_sa.size()? ' ': '\n');
  }

  template <typename InputIt>
  bool M_lexicographical_compare(size_type pos, InputIt first, InputIt last) const {
    while (first != last) {
      if (M_comp(M_raw[pos], *first)) return true;
      if (M_comp(*first, M_raw[pos])) return false;
      ++pos, ++first;
    }
    return false;
  }

  template <typename InputIt>
  InputIt M_mismatch(size_type pos, InputIt first, InputIt last) const {
    while (first != last) {
      if (M_comp(M_raw[pos], *first) || M_comp(*first, M_raw[pos])) break;
      ++pos, ++first;
    }
    return first;
  }

public:
  suffix_array() = default;
  suffix_array(suffix_array const&) = default;
  suffix_array(suffix_array&&) = default;

  template <typename InputIt>
  suffix_array(InputIt first, InputIt last, Compare comp = Compare()):
    M_raw(first, last), M_size(M_raw.size()+1), M_comp(comp)
  {
    M_build();
  }

  template <typename ForwardIt>
  difference_type lcp(ForwardIt first, ForwardIt last) {
    size_type lb = 0;
    size_type ub = M_raw.size();
    while (ub-lb > 1) {
      size_type mid = (lb+ub) >> 1;
      (M_lexicographical_compare(M_sa[mid], first, last)? lb: ub) = mid;
    }
    auto it0 = M_mismatch(M_sa[lb], first, last);
    auto it1 = M_mismatch(M_sa[ub], first, last);
    return std::max(std::distance(first, it0), std::distance(first, it1));
  }

  template <typename ForwardIt>
  bool contains(ForwardIt first, ForwardIt last) {
    return lcp(first, last) == std::distance(first, last);
  }

  template <typename OutputIt>
  OutputIt dump(OutputIt d_first) const {
    return std::copy(M_sa.begin(), M_sa.end(), d_first);
  }
};
