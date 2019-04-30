class bit_vector {
  size_t n;
  static constexpr size_t nbit = 64;
  std::vector<intmax_t> raw;
  std::vector<int> acc;

  int popcount(uintmax_t x) const {
    return __builtin_popcountll(x);
  }

public:
  bit_vector() {}

  bit_vector(const std::vector<bool>& b): n(b.size()) {
    raw.assign(n/nbit+1, 0);
    for (size_t i = 0; i < n; ++i)
      if (b[i]) raw[i/nbit] |= intmax_t(1) << (i % nbit);

    acc.assign(n/nbit+1, 0);
    for (size_t i = 1; i < acc.size(); ++i)
      acc[i] = acc[i-1] + popcount(raw[i-1]);
  }

  size_t rank1(size_t k) const {
    size_t large = k / nbit;
    size_t small = k % nbit;
    size_t res = acc[large];
    if (small > 0) res += popcount(raw[large] & ((uintmax_t(1) << small) - 1));
    return res;
  }

  size_t rank0(size_t k) const {
    return k - rank1(k);
  }

  size_t select1(size_t k) const {
    if (k == 0) return 0;
    size_t lb = 0;
    size_t ub = n;
    while (ub-lb > 1) {
      size_t mid = (lb+ub) >> 1;
      ((rank1(mid) < k)? lb:ub) = mid;
    }
    if (rank1(ub) < k) return -1;
    return ub;
  }

  size_t select0(size_t k) const {
    if (k == 0) return 0;
    size_t lb = 0;
    size_t ub = n;
    while (ub-lb > 1) {
      size_t mid = (lb+ub) >> 1;
      ((rank0(mid) < k)? lb:ub) = mid;
    }
    if (rank0(ub) < k) return -1;
    return ub;
  }

  size_t rank(int x, size_t k) const {
    return x? rank1(k) : rank0(k);
  }

  size_t select(int x, size_t k) const {
    return x? select1(k) : select0(k);
  }

  bool operator [](size_t k) const {
    size_t large = k / nbit;
    size_t small = k % nbit;
    return raw[large] >> small & 1;
  }
};

template <class Tp, size_t bitlen = 8 * sizeof(Tp)>
class wavelet_matrix {
public:
  using value_type = std::make_unsigned<Tp>;

private:
  std::vector<value_type> c;
  std::vector<size_t> zeros;
  size_t n;
  std::array<bit_vector, bitlen> a;

  size_t start_index(value_type x) const {
    size_t s = 0;
    size_t t = 0;
    for (size_t i = bitlen; i-- > 1;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return s;
  }

public:
  template <class ForwardIt>
  wavelet_matrix(ForwardIt first, ForwardIt last):
    c(first, last), zeros(bitlen), n(c.size())
  {
    std::vector<value_type> whole = c;
    for (size_t i = bitlen; i--;) {
      std::vector<value_type> zero, one;
      std::vector<bool> vb(n);
      for (size_t j = 0; j < n; ++j) {
        ((whole[j] >> i & 1)? one:zero).push_back(whole[j]);
        vb[j] = (whole[j] >> i & 1);
      }

      zeros[bitlen-i-1] = zero.size();
      a[bitlen-i-1] = bit_vector(vb);
      if (i == 0) break;
      whole = std::move(zero);
      whole.insert(whole.end(), one.begin(), one.end());
    }
  }

  size_t rank(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return t - s;
  }

  std::pair<bool, value_type> max_lt(value_type x, size_t s, size_t t) const {
    return max_le(x-1, s, t);
  }
  std::pair<bool, value_type> max_le(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 1);
      if (reverted) tg = 0;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 0;

      reverted = false;
      if (tg == 1) {
        if (ok0) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok1) {
          ch = 1;
        } else {
          tight = false;
        }
      } else if (!ok0 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << i) - 1;
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= static_cast<value_type>(1) << i;
      }
    }
    return {true, res};
  }
  std::pair<bool, value_type> min_gt(value_type x, size_t s, size_t t) const {
    return min_ge(x+1, s, t);
  }
  std::pair<bool, value_type> min_ge(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 0);
      if (reverted) tg = 1;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 1;

      reverted = false;
      if (tg == 0) {
        if (ok1) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok0) {
          ch = 0;
        } else {
          tight = false;
        }
      } else if (!ok1 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << ri) - 1;
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= static_cast<value_type>(1) << i;
      }
    }
    return {true, res};
  }

  value_type quantile(size_t k, size_t s, size_t t) const {
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      if (k < z) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        res |= static_cast<value_type>(1) << i;
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        k -= z;
      }
    }
    return res;
  }

  std::array<size_t, 3> rank_three_way(value_type x, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        size_t d = tmp - (t-s);
        eq -= d;
        lt += d;
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
        size_t d = tmp - (t-s);
        eq -= d;
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  std::array<size_t, 3> xored_rank_three_way(value_type x, value_type y, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if ((x ^ y) >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }

      size_t d = tmp - (t-s);
      eq -= d;
      if (y >> i & 1) {
        lt += d;
      } else {
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  size_t select(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t si = start_index(x);
    t += a[bitlen-1].rank(x & 1, si);
    t = a[bitlen-1].select(x & 1, t);

    for (size_t i = 1; i < bitlen; ++i) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) t -= zeros[j];
      t = a[j].select(x >> i & 1, t);
    }

    return t;
  }

  value_type operator [](size_t t) const { return c[t]; }

  void inspect() const {
    for (size_t i = 0; i < bitlen; ++i) {
      fprintf(stderr, "%zu (%zu): ", i, zeros[i]);
      for (size_t j = 0; j < n; ++j)
        fprintf(stderr, "%d%c", a[i][j], j+1<n? ' ':'\n');
    }
  }
};
