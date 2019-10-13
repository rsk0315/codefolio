template <typename Key, typename Compare = std::less<>>
class monotone_minimal_perfect_hash {
public:
  using size_type = size_t;

private:
  mutable std::map<Key, size_type, Compare> M_hf;
  mutable bool M_ready = false;

public:
  monotone_minimal_perfect_hash() = default;
  monotone_minimal_perfect_hash(monotone_minimal_perfect_hash const&) = default;
  monotone_minimal_perfect_hash(monotone_minimal_perfect_hash&&) = default;
  monotone_minimal_perfect_hash& operator =(monotone_minimal_perfect_hash const&) = default;
  monotone_minimal_perfect_hash& operator =(monotone_minimal_perfect_hash&&) = default;

  void push(Key const& k) {
    M_hf[k];
    M_ready = false;
  }
  size_type operator [](Key const& k) const {
    if (!M_ready) {
      size_t i = 0;
      for (auto& p: M_hf) p.second = i++;
      M_ready = true;
    }
    return M_hf.find(k)->second;
  }
  size_type size() const noexcept { return M_hf.size(); }
  auto begin() const { return M_hf.begin(); }
  auto end() const { return M_hf.end(); }
};
