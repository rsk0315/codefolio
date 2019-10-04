class bit_vector {
public:
  using underlying_type = uintmax_t;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  std::vector<underlying_type> M_c;
  static const size_type S_ws = CHAR_BIT * sizeof(underlying_type);

public:
  bit_vector() = default;
  bit_vector(bit_vector const&) = default;
  bit_vector(bit_vector&&) = default;
  template <typename InputIt>
  bit_vector(InputIt first, InputIt last) { assign(first, last); }

  bit_vector& operator =(bit_vector const&) = default;
  bit_vector& operator =(bit_vector&&) = default;

  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    std::vector<bool> tmp(first, last);
    M_c.resize((tmp+S_ws-1) / S_ws);
    for (size_type i = 0; i < tmp.size(); ++i) {
      size_type j0 = i / S_ws;
      size_type j1 = i % S_ws;
      M_c[j0] |= 1_ju << j1;
    }
  }

  size_type rank0(size_type s, size_type t) const;
  size_type rank1(size_type s, size_type t) const;

  size_type select0(size_type s, size_type t) const;
  size_type select1(size_type s, size_type t) const;

  // 愚直ににぶたんして O(log n)-time select なのはよくなさそう．
  // 定数倍の軽めの O(log log n)-time とかができるとうれしい．
};

template <size_t Nb, typename Tp = uintmax_t>
class wavelet_matrix {
public:
  using value_type = Tp;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  std::array<bit_vector, Nb> M_c = {};
  std::array<size_type, Nb> M_z = {};
  enum S_three_way { S_less = 0, S_equal, S_greater };

public:
  wavelet_matrix() = default;
  wavelet_matrix(wavelet_matrix const&) = default;
  wavelet_matrix(wavelet_matrix&&) = default;

  wavelet_matrix& operator =(wavelet_matrix const&) = default;
  wavelet_matrix& operator =(wavelet_matrix&&) = default;

  template <typename InputIt>
  void assign(InputIt first, InputIt last);

  size_type rank(value_type x, size_type s, size_type t) const;
  size_type select(value_type x, size_type n, size_type s) const;

  std::array<size_type, 3> rank_3way(value_type x, size_type s, size_type t) const;
  value_type quantile(size_type k, size_type s, size_type t) const;

  value_type min_greater(value_type x, size_type s, size_type t) const;
  value_type min_greater_equal(value_type x, size_type s, size_type t) const;
  value_type max_less(value_type x, size_type s, size_type t) const;
  value_type max_less_equal(value_type x, size_type s, size_type t) const;

  // {
  //   size_type k = ... rank_3way(x, s, t)[S_xxx];
  //   k = ...;
  //   if (k ...) ...;
  //   return quantile(k, s, t);
  // }

  size_type select_greater(value_type x, size_type n, size_type s) const;
  size_type select_greater_equal(value_type x, size_type n, size_type s) const;
  size_type select_less(value_type x, size_type n, size_type s) const;
  size_type select_less_equal(value_type x, size_type n, size_type s) const;
};
