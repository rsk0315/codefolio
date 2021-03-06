#include <cstdio>
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>

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
  std::vector<Tp> c;
  std::vector<size_t> zeros;
  size_t n;
  std::array<bit_vector, bitlen> a;

  size_t start_index(Tp x) const {
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
    std::vector<Tp> whole = c;
    for (size_t i = bitlen; i--;) {
      std::vector<Tp> zero, one;
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

  size_t rank(Tp x, size_t t) const {
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

  std::pair<bool, Tp> max_lt(Tp x, size_t s, size_t t) const {
    return max_le(x-1, s, t);
  }
  std::pair<bool, Tp> max_le(Tp x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    Tp res = 0;
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
        Tp mask = (Tp(1) << i) - 1;
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
        res |= Tp(1) << i;
      }
    }
    return {true, res};
  }
  std::pair<bool, Tp> min_gt(Tp x, size_t s, size_t t) const {
    return min_ge(x+1, s, t);
  }
  std::pair<bool, Tp> min_ge(Tp x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    Tp res = 0;
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
        Tp mask = (Tp(1) << ri) - 1;  // suspicious
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
        res |= Tp(1) << i;
      }
    }
    return {true, res};
  }

  Tp quantile(size_t k, size_t s, size_t t) const {
    Tp res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      if (k < z) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        res |= Tp(1) << i;
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        k -= z;
      }
    }
    return res;
  }

  std::array<size_t, 3> rank_three_way(Tp x, size_t t) const {
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

  std::array<size_t, 3> xored_rank_three_way(Tp x, Tp y, size_t t) const {
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

  size_t select(Tp x, size_t t) const {
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

  void inspect() const {
    for (size_t i = 0; i < bitlen; ++i) {
      fprintf(stderr, "%zu (%zu): ", i, zeros[i]);
      for (size_t j = 0; j < n; ++j)
        fprintf(stderr, "%d%c", a[i][j], j+1<n? ' ':'\n');
    }
  }
};

#include <cassert>
#include <random>

void test_bv() {
  std::mt19937 rsk(0315);
  size_t n = 16384;
  std::vector<bool> base(n);
  for (size_t i = 0; i < n; ++i) {
    if (rsk() % 2) base[i] = true;
  }

  for (size_t i = 0; i <= 10; ++i)
    fprintf(stderr, "%d%c", !!base[i], i<10? ' ':'\n');

  bit_vector bv(base);
  std::vector<size_t> rank1(n+1);
  for (size_t i = 0; i < n; ++i)
    if (bv[i]) ++rank1[i+1];
  for (size_t i = 1; i <= n; ++i)
    rank1[i] += rank1[i-1];

  std::vector<size_t> select1(n+1, -1);
  select1[0] = 0;
  {
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
      if (bv[i]) select1[++count] = i+1;
    }
  }

  for (size_t i = 0; i <= n; ++i)
    assert(bv.rank1(i) == rank1[i]);

  for (size_t i = 0; i <= n; ++i) {
    fprintf(stderr, "expected: %zu, got: %zu\n", select1[i], bv.select1(i));
    assert(bv.select1(i) == select1[i]);
  }
}

void test_wm() {
  std::mt19937 rsk(0315);

  size_t n = 16384;
  int m = 1024;
  std::vector<int> base(n);
  for (size_t i = 0; i < n; ++i)
    base[i] = rsk() % m;

  // for (size_t i = 0; i < n; ++i)
  //   fprintf(stderr, "%d%c", base[i], i+1<n? ' ':'\n');

  wavelet_matrix<int, 12> wm(base.begin(), base.end());
  for (int j = 0; j < m; ++j) {
    // fprintf(stderr, "%d:\n", j);
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
      // fprintf(stderr, "expects: %zu, got: %zu\n", count, wm.rank(j, i));
      assert(count == wm.rank(j, i));
      if (base[i] == j) {
        ++count;
        // fprintf(stderr, "expects: %zu, got: %zu\n", i+1, wm.select(j, count));
        assert(i+1 == wm.select(j, count));
      }
    }
    // fprintf(stderr, "expects: %zu, got: %zu\n", count, wm.rank(j, n));
    assert(count == wm.rank(j, n));
  }
}

void test_xor() {
  std::mt19937 rsk(0315);

  size_t n = 16384;
  int m = 16;
  std::vector<int> base(n);
  for (size_t i = 0; i < n; ++i)
    base[i] = rsk() % m;

  // std::vector<int> base{0, 1, 2, 3};
  // size_t n = base.size();
  // int m = 4;

  for (size_t i = 0; i < n; ++i)
    fprintf(stderr, "%d%c", base[i], i+1<n? ' ':'\n');

  wavelet_matrix<int, 12> wm(base.begin(), base.end());
  for (int x = 0; x < m; ++x) {
    for (int y = 1; y < m; ++y) {
      // xor with x, compare with y
      std::array<size_t, 3> expected{0, 0, 0};
      for (size_t i = 0; i < n; ++i) {
        if ((base[i] ^ x) > y) {
          ++expected[2];
        } else if ((base[i] ^ x) < y) {
          ++expected[0];
        } else {
          ++expected[1];
        }

        auto got = wm.xored_rank_three_way(x, y, i+1);

        if (got != expected) {
          fprintf(stderr, "expected: [%zu %zu %zu]\n",
                  expected[0], expected[1], expected[2]);

          fprintf(stderr, "got: [%zu %zu %zu]\n",
                  got[0], got[1], got[2]);
          fprintf(stderr, "(x, y, t): (%d, %d, %zu)\n", x, y, i+1);
          assert(got == expected);
        }
      }
    }
  }
}

int main() {
  std::vector<int> a{
    11,  0, 15,  6,
     5,  2,  7, 12,
    11,  0, 12, 12,
    13,  4,  6, 13,
     1, 11,  6,  1,
     7, 10,  2,  7,
    14, 11,  1,  7,
     5,  4, 14,  6
  };

  for (size_t i = 0; i < a.size(); ++i)
    fprintf(stderr, "%d%c", a[i], i+1<a.size()? ' ':'\n');

  wavelet_matrix<int, 5> wm(a.begin(), a.end());
  while (false) {
    // int x;
    // size_t t;
    // if (scanf("%d %zu", &x, &t) != 2) break;
    // printf("select(x:%d, t:%zu): %zu\n", x, t, wm.select(x, t));
    // printf("%zu\n", wm.rank(x, t));
    // auto tw = wm.rank_three_way(x, t);
    // printf("%zu %zu %zu\n", tw[0], tw[1], tw[2]);

    size_t k, s, t;
    if (scanf("%zu %zu %zu", &k, &s, &t) != 3) break;
    printf("%d\n", wm.quantile(k, s, t));
  }

  // test_wm();
  test_xor();
}
