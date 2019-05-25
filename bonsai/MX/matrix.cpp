#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>

template <typename Tp>
class matrix {
public:
  using value_type = Tp;

private:
  class M_container: protected std::vector<Tp> {
    friend matrix<Tp>;
  public:
    M_container() = default;
    M_container(size_t n, const Tp& x): std::vector<Tp>(n, x) {}
    M_container(const std::initializer_list<Tp>& init): std::vector<Tp>(init) {}
    Tp& operator [](size_t i) { return std::vector<Tp>::operator [](i); }
    const Tp& operator [](size_t i) const { return std::vector<Tp>::operator [](i); }
  };

  size_t M_rows = 0;
  size_t M_columns = 0;
  std::vector<M_container> M_c;

public:
  matrix(size_t n, size_t m, const Tp& x):
    M_rows(n), M_columns(m), M_c(n, M_container(m, x))
  {}

  matrix(const std::initializer_list<std::initializer_list<Tp>>& init) {
    for (const auto& ii: init) {
      M_c.emplace_back(ii);
      ++M_rows;
      M_columns = std::max(M_columns, ii.size());
    }
    for (auto& ci: M_c) {
      if (ci.size() < M_columns)
        ci.resize(M_columns, Tp());
    }
  }

  void inspect() const {
    fprintf(stderr, "(%zu, %zu) matrix\n", M_rows, M_columns);
    for (size_t i = 0; i < M_rows; ++i)
      for (size_t j = 0; j < M_columns; ++j)
        std::cerr << M_c[i][j] << (j+1<M_columns? ' ':'\n');
  }

  M_container& operator [](size_t i) { return M_c[i]; }
  const M_container& operator [](size_t i) const { M_c[i]; }

  void clear() { M_c.clear(); }
};

int main() {
  matrix<int> a{{2, 4, 5}, {1}, {3, 5, 7}, {}};

  a.inspect();
  // (4, 3) matrix
  // 2 4 5
  // 1 0 0
  // 3 5 7
  // 0 0 0

  a[0][0] = 1;  // ok
  // a[0].resize(5);  // error
  // a[0].clear();  // error
  a.clear();  // ok
}
