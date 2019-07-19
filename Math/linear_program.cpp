template <typename Tp>
class linear_program {
  // optimize given linear program (in a standard form) with simplex algorithm

public:
  using size_type = size_t;
  using value_type = Tp;

private:
  using matrix_type = std::vector<std::vector<value_type>>;
  using vector_type = std::vector<value_type>;
  using indices_type = std::vector<size_type>;
  using S_nl = std::numeric_limits<value_type>;
  constexpr static bool const S_fp = std::is_floating_point<value_type>::value;
  constexpr static value_type const S_infeasible = S_nl::quiet_NaN();
  constexpr static value_type const S_unbounded = S_nl::infinity();

  // bad
  static bool S_isnan(float x) { return std::isnan(x); }
  static bool S_isnan(double x) { return std::isnan(x); }
  static bool S_isnan(long double x) { return std::isnan(x); }
  static bool S_isnan(value_type const& x) { return x.is_nan(); }
  static bool S_isinf(float x) { return std::isinf(x); }
  static bool S_isinf(double x) { return std::isinf(x); }
  static bool S_isinf(long double x) { return std::isinf(x); }
  static bool S_isinf(value_type const& x) { return x.is_inf(); }

  static void S_pivot(indices_type& ni, indices_type& bi,
                      matrix_type& a, vector_type& b, vector_type& c,
                      value_type& v, size_type l, size_type e) {

    size_type m = b.size();
    size_type n = c.size();

    b[l] /= a[l][e];
    a[l][e] = value_type{1} / a[l][e];
    for (size_type j = 0; j < m; ++j) {
      if (j == e) continue;
      a[l][j] *= a[l][e];
    }

    for (size_type i = 0; i < n; ++i) {
      if (i == l) continue;
      b[i] -= a[i][e] * b[l];
      for (size_type j = 0; j < m; ++j) {
        if (j == e) continue;
        a[i][j] = a[i][j] - a[i][e] * a[l][j];
      }
      a[i][e] = -a[i][e] * a[l][e];
    }
    v += c[e] * b[l];

    for (size_type j = 0; j < n; ++j) {
      if (j == e) continue;
      c[j] -= c[e] * a[l][j];
    }
    c[e] *= -a[l][e];
    std::swap(ni[e], bi[l]);
  }

  static size_type S_any_positive(value_type const& c) {
    for (size_type i = 0; i < c.size(); ++i)
      if (c[i] > 0) return i;
    return -1;
  }

  static value_type S_initialize(indices_type& ni, indices_type& bi,
                                 matrix_type& a, vector_type& b, vector_type& c) {
    size_type m = b.size();
    size_type n = c.size();
    size_type k = std::min_element(b.begin(), b.end()) - b.begin();
    ni.resize(n);
    bi.resize(m);
    std::iota(ni.begin(), ni.end(), 0);
    std::iota(bi.begin(), bi.end(), n);
    if (b[k] >= value_type{0}) {
      // the initial basic solution is feasible
      return value_type{0};
    }

    for (auto& ai: a) ai.emplace_back(-1);
    ni.push_back(n+m);

    vector_type c0(n+1);
    c0[n] = value_type{-1};

    value_type v{};
    S_pivot(ni, bi, a, b, c, v, k, n);  // XXX m? n?
    S_simplex(ni, bi, a, b, c, v);

    for (size_type i = 0; i < bi.size(); ++i) {
      if (bi[i] == n+m) {
        if (b[bi[i]] != value_type{0}) {
          // XXX -Wfloat-equal
          return S_infeasible;
        }
        // perform one (degenerate) pivot to make it nonbasic
        break;
      }
    }
    // from the final slack form of L[aux], remove x[0] from the constraints
    // and restore the original objective function L, but replace each basic
    // variable in this objective function by the right-hand side of its
    // associated constraint
    return /* the modified final slack form */;
  }

  static void S_simplex(indices_type& ni, indices_type& bi,
                        matrix_type& a, vector_type& b, vector_type& c,
                        value_type& v) {

    size_type m = b.size();
    while (true) {
      size_type e = S_any_positive(c);
      if (e+1 == 0) break;

      vector_type delta(m, S_unbounded);
      for (size_type i = 0; i < m; ++i)
        a[i][e] = b[i] / a[i][e];

      auto it = std::min_element(delta.begin(), delta.end());
      if (S_isinf(*it)) return S_unbounded;

      size_type l = it - delta.begin();
      S_pivot(ni, bi, a, b, c, v, l, e);
    }
  }

public:
  linear_program() = delete;

  template <typename Matrix, typename Vector>
  static value_type maximize(Matrix const& a0,
                             Vector const& b0, Vector const& c0) {

    size_type m = b0.size();
    size_type n = c0.size();

    // FIXME better design
    matrix_type a(m);
    for (size_type i = 0; i < m; ++i)
      a[i] = vector_type(a0[i].begin(), a0[i].end());

    vector_type b(b0.begin(), b0.end());
    vector_type c(c0.begin(), c0.end());

    indices_type ni, bi;
    value_type v = S_initialize(ni, bi, a, b, c);
    if (S_isnan(v)) return S_infeasible;

    S_simplex(ni, bi, a, b, c, v);
    return std::accumulate(b.begin(), b.end(), value_type{0});
  }
};

template <typename Tp> Tp const linear_program<Tp>::S_infeasible;
template <typename Tp> Tp const linear_program<Tp>::S_unbounded;
// template <typename Tp> constexpr Tp const linear_program<Tp>::S_fp;
