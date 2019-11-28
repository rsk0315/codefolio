namespace detail {
template <typename Fn>
class totally_monotone_minima {
public:
  using size_type = size_t;
  using value_type = decltype(std::declval<Fn>()(0, 0));
  using function_type = Fn;

private:
  function_type M_f;
  size_type M_h, M_w;
  std::vector<size_type> M_res;

  void M_reduce(std::vector<size_type>& is, std::vector<size_type>& js) {
    assert(is.size() < js.size());
    std::stack<value_type> qy;
    std::deque<size_type> qj;
    auto it = is.begin();
    for (auto j: js) {
      size_type i = *it;
      while (!qy.empty()) {
        if (M_f(i, j) < qy.top()) {
        // if (!(qy.top() < M_f(i, j))) {
          qy.pop();
          qj.pop_back();
          if (it != is.begin()) i = *--it;
        } else {
          ++it;
          break;
        }
      }
      if (it == is.end()) {
        --it;
        continue;
      }
      i = *it;
      qy.push(M_f(i, j));
      qj.push_back(j);
    }
    js.assign(qj.begin(), qj.end());

    assert(is.size() >= js.size());
    M_interpolate(is, js);
  }

  void M_base_step(std::vector<size_type>& is, std::vector<size_type>& js) {
    assert(is.size() <= 2);
    if (is.size() == 1) {
      M_res[is[0]] = js[0];
      return;
    }
    if (js.size() == 1) {
      M_res[is[0]] = M_res[is[1]] = js[0];
      return;
    }
    size_type i0 = is[0];
    size_type i1 = is[1];
    size_type j0 = js[0];
    size_type j1 = js[1];
    M_res[i0] = M_res[i1] = j0;
    if (M_f(i0, j1) < M_f(i0, j0)) {
      M_res[i0] = M_res[i1] = j1;  // from totally-monotonicity
    } else if (M_f(i1, j1) < M_f(i1, j0)) {
      M_res[i1] = j1;
    }
  }

  void M_interpolate(std::vector<size_type>& is, std::vector<size_type>& js) {
    assert(is.size() >= js.size());
    if (is.size() <= 2) {
      M_base_step(is, js);
      return;
    }

    std::vector<size_type> i0, i1;
    i0.reserve((is.size()+1)/2);
    i1.reserve(is.size()/2);
    for (size_type i = 0; i < is.size(); i += 2) i0.push_back(is[i]);
    for (size_type i = 1; i < is.size(); i += 2) i1.push_back(is[i]);
    {
      auto tmp = js;
      M_process(i1, tmp);
    }

    std::queue<size_type> stop;
    for (auto i: i1) stop.push(M_res[i]);
    stop.push(js.back()+1);
    for (auto it = i0.begin(), jt = js.begin(); jt != js.end();) {
      size_type i = *it;
      size_type j = *jt;
      if (M_f(i, j) < M_f(i, M_res[i])) M_res[i] = j;
      if (stop.front() == j) {
        if (++it == i0.end()) break;
        stop.pop();
      } else {
        ++jt;
      }
    }
  }

  void M_build() {
    size_type h = M_h;
    size_type w = M_w;
    std::vector<size_type> is(h), js(w);
    std::iota(is.begin(), is.end(), 0_zu);
    std::iota(js.begin(), js.end(), 0_zu);
    M_process(is, js);
  }

  void M_process(std::vector<size_type>& is, std::vector<size_type>& js) {
    (is.size() < js.size())? M_reduce(is, js): M_interpolate(is, js);
  }

public:
  totally_monotone_minima(function_type f, size_type h, size_type w):
    M_f(f), M_h(h), M_w(w), M_res(h) { M_build(); }

  std::vector<size_type> output() const { return M_res; }
};
}  // detail::

template <typename Fn>
std::vector<size_t> totally_monotone_minima(Fn f, size_t h, size_t w) {
  return detail::totally_monotone_minima<Fn>(f, h, w).output();
}
