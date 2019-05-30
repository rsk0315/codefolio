template <class Tp>
class starry_sky_tree {
  std::vector<Tp> c;

  size_t roundup_powtwo(size_t n) const {
    if (n == 0) return 1;
    if ((n & (n-1)) == 0) return n+1;
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
    c[1] = e;
  }

  template <class ForwardIt>
  starry_sky_tree(ForwardIt first, ForwardIt last) {
    size_t m = 2 * roundup_powtwo(std::distance(first, last));
    c.assign(m, *first-*first);
    for (size_t i = m/2; i < m; ++i) {
      c[i] = *first++;
      if (first == last) break;
    }
    init_build();
  }

  void add(size_t l, size_t r, const Tp& x) {
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
    for (size_t i = l-1; i > 1; i >>= 1)
      maintain(i);
    assert(r < m);
    for (size_t i = r; i >= 1; i >>= 1)
      maintain(i);
  }

  Tp max(size_t l, size_t r) const {
    assert(l < r);
    size_t m = c.size();
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

    std::vector<Tp> cand;
    Tp curl = c[1];
    for (size_t i = li-1; i > 1; i >>= 1) curl += c[i];
    for (size_t i = li-1; !ll.empty();) {
      size_t j = ll.back();
      (i <<= 1) |= 1;
      if (i == j) {
        cand.push_back(curl+c[i--]);
        ll.pop_back();
      }
      curl += c[i];
    }
    Tp curr = c[1];
    for (size_t i = ri; i > 1; i >>= 1) curr += c[i];
    for (size_t i = ri; !rr.empty();) {
      size_t j = rr.back();
      i <<= 1;
      if (i == j) {
        cand.push_back(curr+c[i++]);
        rr.pop_back();
      }
      curr += c[i];
    }
    return *std::max_element(cand.begin(), cand.end());
  }
};
