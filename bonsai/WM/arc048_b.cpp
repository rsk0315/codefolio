#include <cstdio>
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <map>

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
  // static constexpr size_t bitlen = 8 * sizeof(Tp);
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

    // inspect();
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

  Tp max_lt(Tp x, size_t s, size_t t) const;
  Tp max_le(Tp x, size_t s, size_t t) const;
  Tp min_gt(Tp x, size_t s, size_t t) const;
  Tp min_ge(Tp x, size_t s, size_t t) const;
  Tp quantile(size_t k, size_t s, size_t t) const;

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

  size_t select(Tp x, size_t t) const {
    if (t == 0) return 0;
    size_t si = start_index(x);
    t += a[bitlen-1].rank(x & 1, si);
    t = a[bitlen-1].select(x & 1, t);

    for (size_t i = 1; i < bitlen; ++i) {
      // fprintf(stderr, "t: %zu\n", t);
      size_t j = bitlen-i-1;
      if (x >> i & 1) t -= zeros[j];
      t = a[j].select(x >> i & 1, t);
    }

    return t;
  }
};

int main() {
  size_t N;
  scanf("%zu", &N);

  std::vector<int> R(N), H(N);
  std::map<std::pair<int, int>, int> P;
  for (size_t i = 0; i < N; ++i) {
    scanf("%d %d", &R[i], &H[i]);
    ++P[{R[i], H[i] % 3}];
  }

  wavelet_matrix<int, 17> wm(R.begin(), R.end());

  for (size_t i = 0; i < N; ++i) {
    auto tmp = wm.rank_three_way(R[i], N);
    int win = tmp[0];
    int lose = tmp[2];
    win += P[{R[i], (H[i]+1) % 3}];
    lose += P[{R[i], (H[i]+2) % 3}];
    int draw = N - (win+lose+1);

    printf("%d %d %d\n", win, lose, draw);
  }
}
