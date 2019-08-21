template <typename Tp, typename Compare = std::less<Tp>>
class suffix_array {
public:
  using size_type = size_t;
  using value_type = Tp;
  using compare = Compare;

private:
  size_type M_size;
  std::vector<size_type> M_rank;
  std::vector<size_type> M_sa;
  compare M_comp;

  template <typename InputIt>
  void M_build(InputIt first, InputIt last) {
    std::vector<value_type> rank0(first, last);
    M_size = rank0.size() + 1;
    M_sa.resize(M_size);
    std::iota(M_sa.begin(), M_sa.end(), 0_zu);

    {
      auto comp0 = [&](size_type i, size_type j) -> bool {
        if (i == j) return false;
        if (i+1 >= rank0.size()) return true;
        if (j+1 >= rank0.size()) return false;
        // comparison between value_type's, not size_type's
        if (M_comp(rank0[i], rank0[j]) || M_comp(rank0[j], rank0[i]))
          return M_comp(rank0[i], rank0[j]);
        return M_comp(rank0[i+1], rank0[j+1]);
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
  }

public:
  suffix_array() = default;
  suffix_array(suffix_array const&) = default;
  suffix_array(suffix_array&&) = default;

  template <typename InputIt>
  suffix_array(InputIt first, InputIt last, Compare comp = Compare()):
    M_comp(comp) { M_build(first, last); }
};
