template <class Tp>
class lines {
  // based on Li Chao tree

public:
  using size_type = size_t;
  using value_type = Tp;

private:
  std::map<value_type, size_type> enc;
  std::vector<value_type> dec;
  std::vector<std::pair<value_type, value_type>> c;
  size_type m = 1;
  value_type inf = std::numeric_limits<value_type>::max();

  void init_resize(size_type n) {
    while (m < n+n) m <<= 1;
    c.assign(m, {0, inf});
  }

public:
  template <class ForwardIt>
  lines(ForwardIt first, ForwardIt last): dec(first, last) {
    static_assert(std::is_convertible<typename ForwardIt::value_type, value_type>::value, "Type mismatch");
    std::sort(dec.begin(), dec.end());
    size_type n = dec.size();
    init_resize(n);
    // dec.resize(m/2, dec.back());
    while (dec.size() < m/2) dec.push_back(dec.back()+1);
    for (size_type i = 0; i < m/2; ++i) enc.emplace(dec[i], i);
  }

  void append(value_type a, value_type b) {
    size_type l = 0;
    size_type j = 1;
    size_type h = 1;
    size_type w = m/2;
    while (w > 1 && l+w-1 < dec.size()) {
      value_type xl = dec.at(l);
      value_type xm = dec.at(l+w/2);
      value_type xr = dec.at(l+w-1);

      value_type a0, b0;
      std::tie(a0, b0) = c[j];

      value_type yl0 = a0*xl + b0;
      value_type ym0 = a0*xm + b0;
      value_type yr0 = a0*xr + b0;

      value_type yl = a*xl + b;
      value_type ym = a*xm + b;
      value_type yr = a*xr + b;

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
    if (w == 1) {
      value_type a0, b0;
      std::tie(a0, b0) = c[j];
      value_type x = dec.at(l);
      value_type y0 = a0*x + b0;
      value_type y = a*x + b;
      if (y < y0) c[j] = {a, b};
    }
  }

  value_type min_at(value_type x) const {
    size_type i = enc.at(x) + m/2;
    value_type y = inf;
    while (i > 0) {
      value_type a, b;
      std::tie(a, b) = c[i];
      y = std::min(y, a*x+b);
      i >>= 1;
    }
    return y;
  }
};
