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

  static void S_pivot(indices_type& ni, indices_type& bi,
                      matrix_type& a, vector_type& b, vector_type& c,
                      value_type& v, size_type l, size_type e) {

    size_type m = b.size();
    size_type n = c.size();

    b[l] /= a[l][e];
    a[l][e] = value_type{1} / a[l][e];
    for (size_type j = 0; j < n; ++j) {
      if (j == e) continue;
      a[l][j] *= a[l][e];
    }

    for (size_type i = 0; i < m; ++i) {
      if (i == l) continue;
      b[i] -= a[i][e] * b[l];
      for (size_type j = 0; j < n; ++j) {
        if (j == e) continue;
        a[i][j] = a[i][j] - a[i][e] * a[l][j];
      }
      a[i][e] = -a[i][e] * a[l][e];
    }
    v += c[e] * b[l];
    fprintf(stderr, "v <- %.6f\n", v);

    for (size_type j = 0; j < n; ++j) {
      if (j == e) continue;
      c[j] -= c[e] * a[l][j];
    }
    c[e] *= -a[l][e];
    std::swap(ni[e], bi[l]);
  }

  static size_type S_any_positive(vector_type const& c) {
    for (size_type i = 0; i < c.size(); ++i)
      if (c[i] > 0) return i;
    return -1;
  }

  static void S_inspect(matrix_type const& a,
                        vector_type const& b, vector_type const& c,
                        indices_type const& ni, indices_type const& bi) {

    size_type m = b.size();
    size_type n = c.size();
    fprintf(stderr, "---\n");
    fprintf(stderr, "a:\n");
    for (size_type i = 0; i < m; ++i)
      for (size_type j = 0; j < n; ++j)
        fprintf(stderr, "%.3f%c", a[i][j], j+1<n? ' ': '\n');
    fprintf(stderr, "b:\n");
    for (size_type i = 0; i < m; ++i)
      fprintf(stderr, "%.3f%c", b[i], i+1<m? ' ': '\n');
    fprintf(stderr, "c:\n");
    for (size_type j = 0; j < n; ++j)
      fprintf(stderr, "%.3f%c", c[j], j+1<n? ' ': '\n');
    fprintf(stderr, "bi:\n");
    for (size_type i = 0; i < m; ++i)
      fprintf(stderr, "%zu%c", bi[i], i+1<m? ' ': '\n');
    fprintf(stderr, "ni:\n");
    for (size_type j = 0; j < n; ++j)
      fprintf(stderr, "%zu%c", ni[j], j+1<n? ' ': '\n');
    fprintf(stderr, "---\n");
  }

  static value_type S_initialize(indices_type& ni, indices_type& bi,
                                 matrix_type& a, vector_type& b, vector_type& c,
                                 value_type& v) {
    size_type m = b.size();
    size_type n = c.size();
    size_type k = std::min_element(b.begin(), b.end()) - b.begin();
    ni.resize(n);
    bi.resize(m);
    std::iota(ni.begin(), ni.end(), 0);
    std::iota(bi.begin(), bi.end(), n);
    v = value_type{0};
    if (b[k] >= value_type{0}) {
      // the initial basic solution is feasible
      return true;
    }

    for (auto& ai: a) ai.emplace_back(-1);
    ni.push_back(n+m);

    vector_type c0(n+1);
    c0[n] = value_type{-1};

    // S_inspect(a, b, c0, ni, bi);

    S_pivot(ni, bi, a, b, c0, v, k, n);

    // S_inspect(a, b, c, ni, bi);

    S_simplex(ni, bi, a, b, c0, v);

    for (size_type i = 0; i < bi.size(); ++i) {
      if (bi[i] == n+m) {
        fprintf(stderr, "? %.12f\n", b[bi[i]]);
        if (b[bi[i]] != value_type{0}) {
          // XXX -Wfloat-equal
          return false;
        }
        // perform one (degenerate) pivot to make it nonbasic
        S_pivot(ni, bi, a, b, c0, v, i, n);
        break;
      }
    }
    // from the final slack form of L[aux], remove x[0] from the constraints
    // and restore the original objective function L, but replace each basic
    // variable in this objective function by the right-hand side of its
    // associated constraint

    S_inspect(a, b, c0, ni, bi);

    for (size_type j = 0; j <= n; ++j) {
      if (ni[j] != n+m) continue;
      fprintf(stderr, "erasing %zu\n", j);
      for (auto& ai: a) ai.erase(ai.begin()+j);
      c0.erase(c0.begin()+j);
      ni.erase(ni.begin()+j);
      break;
    }

    {
      vector_type c1(n);
      for (size_type j = 0; j < n; ++j) {
        size_type n0 = ni[j];
        if (n0 >= n) continue;
        c1[j] = c[n0];
      }
      for (size_type i = 0; i < m; ++i) {
        size_type b0 = bi[i];
        if (b0 >= n) continue;
        for (size_type j = 0; j < n; ++j) {
          c1[j] -= c[b0] * a[i][j];
        }
        v += c[b0] * b[i];
      }
      c = std::move(c1);
    }

    S_inspect(a, b, c, ni, bi);

    // return /* the modified final slack form */;
    return true;
  }

  static bool S_simplex(indices_type& ni, indices_type& bi,
                        matrix_type& a, vector_type& b, vector_type& c,
                        value_type& v) {

    size_type m = b.size();
    while (true) {
      size_type e = S_any_positive(c);
      if (e+1 == 0) return true;

      vector_type delta(m, S_unbounded);
      for (size_type i = 0; i < m; ++i)
        delta[i] = b[i] / a[i][e];

      auto it = std::min_element(delta.begin(), delta.end());
      if (*it == S_unbounded) return false;

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
    value_type v{};
    if (!S_initialize(ni, bi, a, b, c, v)) return S_infeasible;
    if (!S_simplex(ni, bi, a, b, c, v)) return S_unbounded;
    return v;
  }
};

template <typename Tp> Tp const linear_program<Tp>::S_infeasible;
template <typename Tp> Tp const linear_program<Tp>::S_unbounded;
