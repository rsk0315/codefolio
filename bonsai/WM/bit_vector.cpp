class bit_vector {
public:
  using underlying_type = uintmax_t;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  static const size_type S_ws = CHAR_BIT * sizeof(underlying_type);
  std::vector<underlying_type> M_c;
  std::vector<size_type> M_r;
  std::vector<size_type> M_s0, M_s1;
  std::vector<std::vector<size_type>> M_ss0, M_ss1;

  static size_type S_popcount(underlying_type n) {
    return __builtin_popcountll(n);
  }

  static size_type S_least_n_bits(size_type n) {
    return (1_ju << n) - 1;
  }

  template <int Bp>
  static size_type S_rank_small(underlying_type x, size_type n) {
    if (Bp == 0) x = ~x;
    return S_popcount(x & S_least_n_bits(n));
  }

  template <int Bp>
  static size_type S_select_small(underlying_type x, size_type n) {
    if (n == 0) return 0;
    size_type lb = 0;
    size_type ub = S_ws;
    while (ub-lb > 1) {
      size_type mid = (lb+ub) >> 1;
      ((S_rank_small<Bp>(x, mid) < n)? lb: ub) = mid;
    }
    return ub;
  }

  template <int Bp>
  size_type M_rank_large(size_type n) const {
    // if (n == 0) return 0;
    size_type res = M_r[n];
    if (Bp == 0) res = n * S_ws - res;
    return res;
  }

  template <int Bp>
  void M_prepare_select(std::vector<bool> const& bv,
                        std::vector<size_type>& s,
                        std::vector<std::vector<size_type>>& ss) {
    size_type z = 0;
    size_type n = bv.size();
    s.push_back(0);
    std::vector<size_type> tmp;
    for (size_type i = 0; i < n; ++i) {
      if (bv[i] != Bp) continue;
      tmp.push_back(i);
      if (++z == S_ws) {
        size_type len = i+1 - s.back();
        s.push_back(i+1);
        ss.emplace_back();
        if (len >= S_ws * S_ws) ss.back() = std::move(tmp);
        tmp.clear();
        z = 0;
      }
    }
    ss.push_back(std::move(tmp));
    fprintf(stderr, "s.size(): %zu, ss.size(): %zu\n", s.size(), ss.size());
  }

  size_type M_rank1(size_type t) const {
    size_type j0 = t / S_ws;
    size_type j1 = t % S_ws;
    return M_r[j0] + S_popcount(S_least_n_bits(j1) & M_c[j0]);
  }

  template <int Bp>
  size_type M_select(size_type n,
                     std::vector<size_type> const& s,
                     std::vector<std::vector<size_type>> const& ss) const {

    if (n-- == 0) return 0;
    size_type j0 = n / S_ws;
    size_type j1 = n % S_ws;

    if (j0 >= s.size()) return -1_zu;
    if (j0+1 == s.size() && j1 >= ss[j0].size()) return -1_zu;
// #warning "for debug"
    if (!ss[j0].empty()) return ss[j0][j1] + 1;

    size_type lb = s[j0] / S_ws;
    size_type ub = (j0+1 < s.size())? (s[j0+1]+S_ws-1) / S_ws: M_r.size();
    fprintf(stderr, "\nn: %zu, lb: %zu, ub: %zu\n", n, lb, ub);
    while (ub-lb > 1) {
      size_type mid = (lb+ub) >> 1;
      ((M_rank_large<Bp>(mid) <= n)? lb: ub) = mid;
    }
    return lb * S_ws + S_select_small<Bp>(M_c[lb], n+1 - M_rank_large<Bp>(lb));
  }

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
    M_c.resize(tmp.size() / S_ws + 1);
    for (size_type i = 0; i < tmp.size(); ++i) {
      if (!tmp[i]) continue;
      size_type j0 = i / S_ws;
      size_type j1 = i % S_ws;
      M_c[j0] |= 1_ju << j1;
    }

    // rank
    M_r.assign(M_c.size(), 0);
    for (size_type i = 1; i < M_c.size(); ++i)
      M_r[i] += M_r[i-1] + S_popcount(M_c[i-1]);

    // select
    M_prepare_select<0>(tmp, M_s0, M_ss0);
    M_prepare_select<1>(tmp, M_s1, M_ss1);
  }

  size_type rank0(size_type s, size_type t) const {
    return (t-s) - rank1(s, t);
  }
  size_type rank1(size_type s, size_type t) const {
    if (s == t) return 0;
    return M_rank1(t) - M_rank1(s);
  }
  size_type select0(size_type n, size_type s) const {
    n += rank0(0, s);
    return M_select<0>(n, M_s0, M_ss0);
  }
  size_type select1(size_type n, size_type s) const {
    n += rank1(0, s);
    return M_select<1>(n, M_s1, M_ss1);
  }
};

template <size_t Nb, typename Tp = uintmax_t>
class wavelet_matrix {
public:
  using value_type = Tp;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  std::array<bit_vector, Nb> M_a = {};
  std::array<size_type, Nb> M_z = {};
  std::vector<value_type> M_c;
  enum S_three_way { S_less = 0, S_equal, S_greater };
  static const value_type S_fail = -1;  // XXX use std::optional

  size_type M_startpos(value_type x) const {
    size_type s = 0;
    size_type t = 0;
    for (size_type i = Nb; i-- > 1;) {
      size_type j = Nb-i-1;
      if (x >> i & 1) {
        s = M_z[j] + M_a[j].rank1(s);
        t = M_z[j] + M_a[j].rank1(t);
      } else {
        s = M_a[j].rank0(s);
        t = M_a[j].rank0(t);
      }
    }
    return s;
  }

public:
  wavelet_matrix() = default;
  wavelet_matrix(wavelet_matrix const&) = default;
  wavelet_matrix(wavelet_matrix&&) = default;

  template <typename InputIt>
  wavelet_matrix(InputIt first, InputIt last) {
    assign(first, last);
  }
  wavelet_matrix(std::initializer_list<value_type> il):
    wavelet_matrix(il.begin(), il.end())
  {}

  wavelet_matrix& operator =(wavelet_matrix const&) = default;
  wavelet_matrix& operator =(wavelet_matrix&&) = default;

  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    M_c.assign(first, last);
    M_z = {{}};
    size_type n = M_c.size();
    std::vector<value_type> whole = M_c;
    for (size_type i = Nb; i--;) {
      std::vector<value_type> zero, one;
      std::vector<bool> vb(n);
      for (size_type j = 0; j < n; ++j) {
        ((whole[j] >> i & 1)? one: zero).push_back(whole[j]);
        vb[j] = (whole[j] >> i & 1);
      }

      M_z[Nb-i-1] = zero.size();
      M_a[Nb-i-1] = bit_vector(vb.begin(), vb.end());
      if (i == 0) break;
      whole = std::move(zero);
      whole.insert(whole.end(), one.begin(), one.end());
    }
  }

  size_type rank(value_type x, size_type s, size_type t) const {
    if (s == t) return 0;
    for (size_type i = Nb; i--;) {
      size_type j = Nb-i-1;
      if (x >> i & 1) {
        s = M_z[j] + M_a[j].rank1(s);
        t = M_z[j] + M_a[j].rank1(t);
      } else {
        s = M_a[j].rank0(s);
        t = M_a[j].rank0(t);
      }
    }
    return t - s;
  }

  size_type select(value_type x, size_type n) const {
    if (n == 0) return 0;
    if (rank(x, 0, M_a.size()) < n) return -1;
    size_type si = M_startpos(x);
    n += M_a[Nb-1].rank(x & 1, si);
    n = M_a[Nb-1].select(x & 1, n);

    for (size_type i = 1; i < Nb; ++i) {
      size_type j = Nb-i-1;
      if (x >> i & 1) n -= M_z[j];
      n = M_a[j].select(x >> i & 1, n);
    }
    return n;
  }
  size_type select(value_type x, size_type n, size_type s) const {
    if (n == 0) return s;
    n += rank(x, 0, s);
    return select(x, n);
  }

  std::array<size_type, 3> rank_3way(value_type x,
                                     size_type s, size_type t) const {

    if (s == t) return {0, 0, 0};

    size_type lt = 0;
    size_type eq = t-s;
    size_type gt = 0;
    for (size_type i = Nb; i--;) {
      size_type j = Nb-i-1;
      size_type tmp = t-s;
      if (x >> i & 1) {
        s = M_z[j] + M_a[j].rank1(s);
        t = M_z[j] + M_a[j].rank1(t);
      } else {
        s = M_a[j].rank0(s);
        t = M_a[j].rank0(t);
      }
      size_type d = tmp - (t-s);
      eq -= d;
      ((x >> i & 1)? lt: gt) += d;
    }
    return {lt, eq, gt};
  }

  std::array<size_type, 3> xored_rank_3way(value_type x, value_type y,
                                           size_type s, size_type t) const {

    if (s == t) return {0, 0, 0};

    size_type lt = 0;
    size_type eq = t-s;
    size_type gt = 0;
    for (size_type i = Nb; i--;) {
      size_type j = Nb-i-1;
      size_type tmp = t-s;
      if ((x ^ y) >> i & 1) {
        s = M_z[j] + M_a[j].rank1(s);
        t = M_z[j] + M_a[j].rank1(t);
      } else {
        s = M_a[j].rank0(s);
        t = M_a[j].rank0(t);
      }

      size_type d = tmp - (t-s);
      eq -= d;
      ((y >> i & 1)? lt: gt) += d;
    }
    return {lt, eq, gt};
  }

  value_type quantile(size_type k, size_type s, size_type t) const {
    value_type res = 0;
    for (size_type i = Nb; i--;) {
      size_type j = Nb-i-1;
      size_type z = M_a[j].rank0(t) - M_a[j].rank0(s);
      if (k < z) {
        s = M_a[j].rank0(s);
        t = M_a[j].rank0(t);
      } else {
        res |= 1_ju << i;
        s = M_z[j] + M_a[j].rank1(s);
        t = M_z[j] + M_a[j].rank1(t);
        k -= z;
      }
    }
    return res;
  }

  value_type min_greater(value_type x, size_type s, size_type t) const {
    auto r3 = rank_3way(x, s, t);
    size_type k = r3[S_less] + r3[S_equal];
    if (k == t-s) return S_fail;
    return quantile(k, s, t);
  }
  value_type min_greater_equal(value_type x, size_type s, size_type t) const {
    auto r3 = rank_3way(x, s, t);
    size_type k = r3[S_less];
    if (k == t-s) return S_fail;
    return quantile(k, s, t);
  }
  value_type max_less(value_type x, size_type s, size_type t) const {
    auto r3 = rank_3way(x, s, t);
    size_type k = r3[S_less];
    if (k == 0) return S_fail;
    return quantile(k-1, s, t);
  }
  value_type max_less_equal(value_type x, size_type s, size_type t) const {
    auto r3 = rank_3way(x, s, t);
    size_type k = r3[S_less] + r3[S_equal];
    if (k == 0) return S_fail;
    return quantile(k-1, s, t);
  }

  size_type select_greater(value_type x, size_type n, size_type s) const;
  size_type select_greater_equal(value_type x, size_type n, size_type s) const;
  size_type select_less(value_type x, size_type n, size_type s) const;
  size_type select_less_equal(value_type x, size_type n, size_type s) const;
};
