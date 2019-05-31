template <typename Tp>
class matrix {
public:
  using value_type = Tp;

private:
  class M_container: private std::vector<Tp> {
    friend matrix<Tp>;
  public:
    M_container() = default;
    M_container(size_t n, Tp const& x = Tp{}): std::vector<Tp>(n, x) {}
    M_container(const std::initializer_list<Tp>& init): std::vector<Tp>(init) {}
    Tp& operator [](size_t i) { return std::vector<Tp>::operator [](i); }
    Tp const& operator [](size_t i) const { return std::vector<Tp>::operator [](i); }
  };

  size_t M_rows = 0;
  size_t M_columns = 0;
  std::vector<M_container> M_c;

  matrix M_modmul(matrix const& rhs, Tp mod) const {
    if (M_columns != rhs.M_rows) throw std::logic_error("size mismatch");

    matrix res(M_rows, rhs.M_columns);
    for (size_t i = 0; i < M_rows; ++i)
      for (size_t j = 0; j < M_columns; ++j)
        for (size_t k = 0; k < rhs.M_columns; ++k)
          (res[i][k] += (*this)[i][j] * rhs[j][k]) %= mod;

    return res;
  }

public:
  matrix(size_t n, size_t m, Tp const& x = Tp{}):
    M_rows(n), M_columns(m), M_c(n, M_container(m, x))
  {}

  matrix(std::initializer_list<std::initializer_list<Tp>> const& init) {
    for (auto const& ii: init) {
      M_c.emplace_back(ii);
      ++M_rows;
      M_columns = std::max(M_columns, ii.size());
    }
    for (auto& ci: M_c) {
      if (ci.size() < M_columns)
        ci.resize(M_columns, Tp());
    }
  }

  M_container& operator [](size_t i) { return M_c[i]; }
  M_container const& operator [](size_t i) const { return M_c[i]; }

  matrix modpow(intmax_t iexp, Tp mod) {
    if (M_rows != M_columns) throw std::logic_error("non-square matrix");

    size_t n = M_rows;
    matrix eye(n, n);
    for (size_t i = 0; i < n; ++i) eye[i][i] = Tp{1};
    matrix res = eye;
    for (matrix dbl = *this; iexp; iexp >>= 1) {
      if (iexp & 1) res = res.M_modmul(dbl, mod);
      dbl = dbl.M_modmul(dbl, mod);
    }
    return res;
  }

  void clear() { M_c.clear(); }
};
