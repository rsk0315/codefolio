#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <array>

#include <random>

class BitVector {
  size_t n, lg_n;
  size_t large_width, small_width, pop_width;
  // small を <int> で実装するなと書いてあるが？
  std::vector<size_t> small, popcount;
  std::vector<int> original;

  size_t lg(size_t n) const {
    return n? 63-__builtin_clzll(n):0;
  }

public:
  BitVector(): n(0), lg_n(0), small(), popcount() {}

  BitVector(const std::vector<bool> &v):
    // ここの n が怖くて，適当に切り上げる必要がありそう
    n(v.size()), lg_n(lg(n)),
    small_width(lg_n/2), pop_width(1<<small_width),
    small(n/small_width), popcount(pop_width),
    original(n/small_width)
  {
    // /* SMALL */
    for (size_t i=0; i+small_width<n; ++i)
      // ここ二重ループにするとかで除算を回避したい
      if (v[i]) ++small[i/small_width+1];

    for (size_t i=1; i<n/small_width; ++i)
      small[i] += small[i-1];

    /* POP-COUNT */
    for (size_t i=0; i<pop_width; ++i)
      popcount[i] = __builtin_popcount(i);

    /* ORIGINAL */
    for (size_t i=0; i<n; i+=small_width)
      for (size_t j=0; j<small_width; ++j)
        if (v[i+j]) original[i/small_width] |= 1<<j;
  }

  size_t rank(size_t k) const {
    if (k-- == 0) return 0;
    int res=popcount[original[k/small_width]&((1<<(k%small_width+1))-1)];
    res += small[k/small_width];
    return res;
  }

  size_t select(size_t m) const {
    if (m == 0) return 0;

    size_t lb=0, ub=small.size();
    while (ub-lb > 1) {
      size_t mid=(lb+ub)>>1;
      (small[mid]<m? lb:ub) = mid;
    }
    for (size_t i=0; i<small_width; ++i) {
      if (original[lb] >> i & 1) {
        if (--m == small[lb])
          return lb*small_width+i+1;
      }
    }

    return -1;
  }
};

template <class Integer>
class WaveletMatrix {
  std::vector<Integer> entity;
  static constexpr size_t BIT_SIZE=8*sizeof(Integer);
  std::array<BitVector, BIT_SIZE> bv;

public:
  template <class ForwardIt>
  WaveletMatrix(ForwardIt first, ForwardIt last) {
    size_t n=last-first;
    std::vector<Integer> vi(first, last);
    for (size_t i=BIT_SIZE; i--;) {
      std::vector<Integer> one, zero;
      std::vector<bool> vb(n);
      for (size_t j=0; j<vi.size(); ++j) {
        if (vi[j]>>i & 1) {
          vb[j] = true;
          one.push_back(vi[j]);
        } else {
          zero.push_back(vi[j]);
        }
      }
      bv[i] = BitVector(vb);

      vi = std::move(zero);
      vi.insert(vi.end(), one.begin(), one.end());  // ???

      if (i <= 3 && false) {
        for (size_t j=0; j<n; ++j)
          fprintf(stderr, "%2d%c", vb[j]&1, j+1<n? ' ':'\n');

        for (size_t j=0; j<n; ++j)
          fprintf(stderr, "%2d%c", vi[j], j+1<n? ' ':'\n');

        fprintf(stderr, "\n");
      }
    }
  }
};

int main() {
  std::vector<int> a{
    11, 0, 15, 6, 5, 2, 7, 12,
    11, 0, 12, 12, 13, 4, 6, 13,
    1, 11, 6, 1, 7, 10, 2, 7,
    14, 11, 1, 7, 5, 4, 14, 6
  };
  WaveletMatrix<int> wm(a.begin(), a.end());
}

// std::mt19937 rsk(0315);

// int main() {
//   std::vector<bool> v(256);
//   for (size_t i=0; i<v.size(); ++i)
//     v[i] = rsk() & 1;

//   BitVector bv(v);

//   while (true) {
//     for (size_t i=0; i<v.size(); ++i) {
//       if (i % 8 == 0) {
//         fprintf(stderr, "\x1b[01;31m");
//       } else if (i % 8 == 4) {
//         fprintf(stderr, "\x1b[0m");
//       }
//       if (i > 0 && i % 64 == 0) {
//         fprintf(stderr, "\n");
//       }
//       fprintf(stderr, "%d", v[i]&1);
//     }
//     fprintf(stderr, "\n");

//     char op;
//     scanf(" %c", &op);
    
//     if (op == 's') {
//       size_t m;
//       scanf("%zu", &m);
//       printf("%zu\n", bv.select(m));
//     } else if (op == 'r') {
//       size_t k;
//       scanf("%zu", &k);
//       printf("%zu\n", bv.rank(k));
//     }
//   }
// }
