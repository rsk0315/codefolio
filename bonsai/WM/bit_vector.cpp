class bit_vector {
public:
  using underlying_type = uintmax_t;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  std::vector<underlying_type> M_c;

public:
  bit_vector() = default;
  bit_vector(bit_vector const&) = default;
  bit_vector(bit_vector&&) = default;

  bit_vector& operator =(bit_vector const&) = default;
  bit_vector& operator =(bit_vector&&) = default;

  size_type rank0(size_type s, size_type t) const;
  size_type rank1(size_type s, size_type t) const;

  size_type select0(size_type s, size_type t) const;
  size_type select1(size_type s, size_type t) const;
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
  size_type select(value_type x, size_type s, size_type t) const;

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

  size_type argmin_greater(value_type x, size_type s, size_type t) const;
  size_type argmin_greater_equal(value_type x, size_type s, size_type t) const;
  size_type argmax_less(value_type x, size_type s, size_type t) const;
  size_type argmax_less_equal(value_type x, size_type s, size_type t) const;

  // {
  //   rank_3way();
  //   quantile();
  //   rank();
  //   select();
  // }
};
