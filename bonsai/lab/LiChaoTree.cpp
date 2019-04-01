#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <type_traits>
#include <limits>
#include <tuple>

template <class Tp>
class lines {
  // based on Li Chao tree
  std::map<Tp, size_t> enc;
  std::vector<Tp> dec;
  std::vector<std::pair<Tp, Tp>> c;
  size_t m = 1;
  Tp inf = std::numeric_limits<Tp>::max() / 8;

private:
  void init_resize(size_t n) {
    while (m < n+n) m <<= 1;
    c.assign(m, {0, inf});
  }

  void inspect() const {
    for (size_t i = 1; i < m; ++i) {
      std::string s = (c[i].first < inf? std::to_string(c[i].first) : "inf");
      std::string t = (c[i].second < inf? std::to_string(c[i].second) : "inf");
      fprintf(stderr, "(%s, %s)%c", s.c_str(), t.c_str(), (i&(i+1))? ' ':'\n');
    }
  }

public:
  template <class ForwardIt>
  lines(ForwardIt first, ForwardIt last): dec(first, last) {
    static_assert(std::is_same<typename ForwardIt::value_type, Tp>::value, "Type mismatch");
    std::sort(dec.begin(), dec.end());
    size_t n = dec.size();
    init_resize(n);
    dec.resize(m/2, dec.back());
    for (size_t i = 0; i < m/2; ++i) enc.emplace(dec[i], i);
  }

  void append(Tp a, Tp b) {
    size_t l = 0;
    size_t j = 1;
    size_t h = 1;
    size_t w = m/2;
    while (w > 1 && l+w-1 < dec.size()) {
      Tp xl = dec.at(l);
      Tp xm = dec.at(l+w/2);
      Tp xr = dec.at(l+w-1);

      Tp a0, b0;
      std::tie(a0, b0) = c[j];

      Tp yl0 = a0*xl + b0;
      Tp ym0 = a0*xm + b0;
      Tp yr0 = a0*xr + b0;

      Tp yl = a*xl + b;
      Tp ym = a*xm + b;
      Tp yr = a*xr + b;

      if (yl0 <= yl && yr0 <= yr) break;
      if (yl0 >= yl && yr0 >= yr) {
        c[j] = {a, b};
        break;
      }

      h <<= 1;
      w >>= 1;

      if (ym0 >= ym) {
        c[j] = {a, b};
        a = a0;
        b = b0;
        j <<= 1;
        if (yr0 < yr) {
          j |= 1;
          l += w;
        }
      } else {
        j <<= 1;
        if (yr0 >= yr) {
          j |= 1;
          l += w;
        }
      }
    }
  }

  Tp min_at(Tp x) const {
    size_t i = enc.at(x) + m/2;
    Tp y = inf;
    while (i > 0) {
      Tp a, b;
      std::tie(a, b) = c[i];
      y = std::min(y, a*x+b);
      i >>= 1;
    }
    return y;
  }
};

int main() {
  size_t N;
  intmax_t C;
  scanf("%zu %jd", &N, &C);

  std::vector<intmax_t> h(N);
  for (auto& hi: h) scanf("%jd", &hi);

  lines<intmax_t> cht(h.begin(), h.end());

  std::vector<intmax_t> dp(N);
  for (size_t i = 0; i+1 < N; ++i) {
    cht.append(-2*h[i], h[i]*h[i]+dp[i]);
    dp[i+1] = cht.min_at(h[i+1]) + h[i+1]*h[i+1] + C;
  }

  printf("%jd\n", dp[N-1]);
}
