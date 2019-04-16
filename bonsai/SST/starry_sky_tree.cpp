#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>

template <class Tp>
class starry_sky_tree {
  std::vector<Tp> c;
  std::vector<Tp> naive;

  size_t roundup_powtwo(size_t n) const {
    if (n == 0) return 1;
    if ((n & (n-1)) == 0) return n;
    return size_t(1) << (64 - __builtin_clzll(n));
  }

  void init_build() {
    size_t m = c.size();
    for (size_t i = m/2; i-- > 1;) {
      Tp max = std::max(c[i<<1], c[i<<1|1]);
      c[i<<1] -= max;
      c[i<<1|1] -= max;
      c[i] = max;
    }
  }

  void maintain(size_t i) {
    Tp max = std::max(c[i<<1], c[i<<1|1]);
    c[i<<1] -= max;
    c[i<<1|1] -= max;
    c[i] += max;
  }

public:
  starry_sky_tree() {}

  starry_sky_tree(size_t n, const Tp& e = Tp()) {
    size_t m = 2 * roundup_powtwo(n);
    c.assign(m, e-e);
    naive.assign(n, e);
    c[1] = e;
  }

  template <class ForwardIt>
  starry_sky_tree(ForwardIt first, ForwardIt last) {
    size_t m = 2 * roundup_powtwo(std::distance(first, last));
    c.assign(m, *first-*first);
    for (size_t i = m/2; i < m; ++i) {
      naive[i-m-2] = *first;
      c[i] = *first++;
      if (first == last) break;
    }
    init_build();
  }

  void add(size_t l, size_t r, const Tp& x) {
    for (size_t i = l; i < r; ++i) naive[i] += x;
    size_t m = c.size();
    l += m/2;
    r += m/2;
    while (l < r) {
      if (l & 1) {
        c[l] += x;
        ++l;
      }
      if (r & 1) {
        --r;
        c[r] += x;
      }
      if ((l-1) > 1) maintain((l-1)>>1);
      if (r < m) maintain(r>>1);
      l >>= 1;
      r >>= 1;
    }
    fprintf(stderr, "l: %zu, r: %zu\n", l, r);
    for (size_t i = l-1; i > 1; i >>= 1)
      maintain(i);
    assert(r < m);
    for (size_t i = r; i >= 1; i >>= 1)
      maintain(i);
  }

  Tp max(size_t l, size_t r) const {
    assert(l < r);
    size_t m = c.size();
    Tp nres = *std::max_element(naive.begin()+l, naive.begin()+r);
    l += m/2;
    r += m/2;
    std::vector<size_t> ll, rr;
    size_t li = l;
    size_t ri = r;
    while (li < ri) {
      if (li & 1) ll.push_back(li++);
      if (ri & 1) rr.push_back(--ri);
      li >>= 1;
      ri >>= 1;
    }

    fprintf(stderr, "%zu:%zu\n", ll.size(), rr.size());
    for (size_t i = 0; i < ll.size(); ++i)
      fprintf(stderr, "%zu%c", ll[i], i+1<ll.size()? ' ':'\n');
    for (size_t i = 0; i < rr.size(); ++i)
      fprintf(stderr, "%zu%c", rr[i], i+1<rr.size()? ' ':'\n');

    std::vector<Tp> cand;
    Tp curl = c[1];
    for (size_t i = li-1; i > 1; i >>= 1) curl += c[i];
    for (size_t i = li-1; !ll.empty();) {
      fprintf(stderr, "l: %zu\n", i);
      size_t j = ll.back();
      (i <<= 1) |= 1;
      if (i == j) {
        cand.push_back(curl+c[i--]);
        fprintf(stderr, "lx: %d\n", cand.back());
        ll.pop_back();
      }
      curl += c[i];
      fprintf(stderr, "curl: %d\n", curl);
    }
    Tp curr = c[1];
    for (size_t i = ri; i > 1; i >>= 1) curr += c[i];
    for (size_t i = ri; !rr.empty();) {
      fprintf(stderr, "r: %zu\n", i);
      size_t j = rr.back();
      i <<= 1;
      if (i == j) {
        fprintf(stderr, "+c[%zu]: %d\n", i, curr+c[i]);
        cand.push_back(curr+c[i++]);
        fprintf(stderr, "rx: %d\n", cand.back());
        rr.pop_back();
      }
      curr += c[i];
      fprintf(stderr, "curr: %d\n", curr);
    }
    Tp res = *std::max_element(cand.begin(), cand.end());
    assert(res == nres);
    return res;
  }

  void inspect() const {
    for (size_t i = 1; i < c.size(); ++i)
      fprintf(stderr, "%d%c", c[i], (i&(i+1))? ' ':'\n');
  }

  void validate() const {
    size_t m = c.size();
    for (size_t i = 0; i < naive.size(); ++i) {
      Tp res = c[1];
      for (size_t j = i+m/2; j > 1; j >>= 1)
        res += c[j];
      assert(res == naive[i]);
    }

    Tp e = c[1] - c[1];
    for (size_t i = 1; i < c.size()/2; ++i)
      assert(c[i<<1] == e || c[i<<1|1] == e);
  }
};

int main() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);

  starry_sky_tree<int> sst(n);
  for (size_t i = 0; i < q; ++i) {
    int c;
    scanf("%d", &c);

    if (c == 0) {
      // add(s, t, x);
      size_t s, t;
      int x;
      scanf("%zu %zu %d", &s, &t, &x);
      sst.add(s, t+1, -x);
    } else if (c == 1) {
      // find(s, t);
      size_t s, t;
      scanf("%zu %zu", &s, &t);
      printf("%d\n", -sst.max(s, t+1));
    }
  }
}

// #include <random>

// int main() {
//   size_t n = 8;
//   starry_sky_tree<int> sst(n);

//   std::mt19937 rsk(0315);
//   for (size_t i = 0; i < 2000; ++i) {
//     size_t l = rsk() % n;
//     size_t r = (l + rsk() % (n-1) + 1) % n;
//     if (l > r) std::swap(l, r);

//     int x = rsk() % 2000;
//     x -= 1000;
//     sst.add(l, r, x);

//     fprintf(stderr, "+ [%zu, %zu): %d\n", l, r, x);
//     sst.inspect();
//     sst.validate();
//     fprintf(stderr, "================================================================\n");

//     ///
//     {
//       size_t l = rsk() % n;
//       size_t r = (l + rsk() % (n-1) + 1) % n;
//       if (l > r) std::swap(l, r);

//       fprintf(stderr, "max in [%zu, %zu): ", l, r);
//       int x = sst.max(l, r);
//       fprintf(stderr, "%d\n", x);
//     }
//   }
// }
