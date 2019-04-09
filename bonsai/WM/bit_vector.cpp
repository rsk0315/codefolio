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

  int popcount(intmax_t x) const {
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
    if (small > 0) res += popcount(raw[large] & ((intmax_t(1) << small) - 1));
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

  size_t rank(size_t k, int x) const {
    return x? rank1(k) : rank0(k);
  }

  size_t select(size_t k, int x) const {
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
  // static constexpr size_t bitlen = 8 * sizeof(Tp);
  std::vector<Tp> c;
  std::vector<size_t> zeros;
  size_t n;
  std::array<bit_vector, bitlen+1> a;

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
      if (i == 0) {
        std::vector<bool> vb(n);
        for (size_t j = zero.size(); j < n; ++j) vb[j] = true;
        a[bitlen] = bit_vector(vb);
        break;
      }
      whole = std::move(zero);
      whole.insert(whole.end(), one.begin(), one.end());
    }

    inspect();
  }

  size_t rank(size_t t, Tp x) const {
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

  size_t select(size_t t, Tp x) const {
    for (size_t i = 0; i < bitlen; ++i) {
      fprintf(stderr, "%zu\n", t);
      size_t j = bitlen-i;
      if (x >> i & 1) {
        size_t ti = zeros[j-1];
        size_t tr = a[j].rank1(ti);
        fprintf(stderr, "ti: %zu, tr: %zu\n", ti, tr);
        t = a[j].select1(t+tr) - ti;
      } else {
        t = a[j].select0(t);
      }
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

void test() {
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

  wavelet_matrix<int> wm(a.begin(), a.end());
  while (true) {
    size_t t;
    int x;
    if (scanf("%zu %d", &t, &x) != 2) break;
    printf("%zu\n", wm.select(t, x));
  }
}
