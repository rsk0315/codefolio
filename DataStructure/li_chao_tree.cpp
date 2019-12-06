template <typename Tp, Tp inf = std::numeric_limits<Tp>::max()>
class li_chao_tree {
public:
  using value_type = Tp;
  using size_type = size_t;

private:
  using line_type = std::pair<value_type, value_type>;
  std::vector<value_type> M_x;
  size_type M_n;
  std::vector<line_type> M_l;

  static const value_type S_inf = inf;

  value_type M_f(size_type i, value_type x) const {
    x *= M_l[i].first;
    x += M_l[i].second;
    return x;
  }

  void M_descend(size_type i, value_type a, value_type b) {
    size_type l = i;
    size_type r = i+1;
    while (l < M_n) {
      l <<= 1;
      r <<= 1;
    }

    while (r-l > 1) {
      size_type m = (l+r) >> 1;
      value_type xl = M_x[l-M_n];
      value_type xm = M_x[m-M_n];
      value_type xr = M_x[r-1-M_n];
      value_type yl0 = M_f(i, xl);
      value_type ym0 = M_f(i, xm);
      value_type yr0 = M_f(i, xr);
      value_type yl1 = a * xl + b;
      value_type ym1 = a * xm + b;
      value_type yr1 = a * xr + b;

      if (yl0 < yl1 && yr0 < yr1) return;
      if (yl1 < yl0 && yr1 < yr0) {
        M_l[i] = {a, b};
        return;
      }

      if (ym1 < ym0) {
        std::swap(M_l[i].first, a);
        std::swap(M_l[i].second, b);
      }
      if ((yl0 < yl1 && yr1 < yr0) ^ (ym1 < ym0)) {
        l = m;
        i = (i << 1 | 1);
      } else {
        r = m;
        i = (i << 1 | 0);
      }
    }
    value_type xl = M_x[l-M_n];
    if (a * xl + b < M_f(l, xl))
      M_l[l] = {a, b};
  }

public:
  li_chao_tree() = default;
  li_chao_tree(li_chao_tree const&) = default;
  li_chao_tree(li_chao_tree&&) = default;

  template <typename InputIt>
  li_chao_tree(InputIt first, InputIt last):
    M_x(first, last), M_n(M_x.size()),
    M_l(M_n*2, std::make_pair(value_type(0), S_inf))
  {}  // assuming std::is_sorted(first, last);

  li_chao_tree& operator =(li_chao_tree const&) = default;
  li_chao_tree& operator =(li_chao_tree&&) = default;

  void push(value_type const& a, value_type const& b) {
    push(a, b, 0, M_n);
  }

  void push(value_type const& a, value_type const& b, size_type l, size_type r) {
    l += M_n;
    r += M_n;
    while (l < r) {
      if (l & 1) M_descend(l++, a, b);
      if (r & 1) M_descend(--r, a, b);
      l >>= 1;
      r >>= 1;
    }
  }

  value_type get(size_type i) const {
    value_type x = M_x[i];
    value_type res = S_inf;
    i += M_n;
    while (i > 0) {
      res = std::min(res, M_f(i, x));
      i >>= 1;
    }
    return res;
  }
};

template <typename Tp, Tp x>
Tp const li_chao_tree<Tp, x>::S_inf;
