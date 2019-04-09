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

  bool operator [](size_t k) const {
    size_t large = k / nbit;
    size_t small = k % nbit;
    return raw[large] >> small & 1;
  }
};

template <class Tp>
class wavelet_matrix {
  std::vector<Tp> c;
  static constexpr size_t bitlen = 8 * sizeof(Tp);
  std::array<bit_vector, bitlen> a;
};

#include <cassert>
#include <random>

int test() {
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
