#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>

template <class BidirIt>
class fractional_cascading {
  using container_type = typename BidirIt::value_type;
  using iterator_type = typename container_type::iterator;
  using value_type = typename container_type::value_type;
  struct node_type {
    const value_type& value;
    size_t current, next;
    node_type(const value_type& value, size_t current, size_t next): value(value), current(current), next(next) {}
  };

  std::vector<std::vector<node_type>> body;

public:
  fractional_cascading(BidirIt first, BidirIt last) {
    size_t n = last - first;
    body.resize(n+1);

    for (size_t i = n; i--;) {
      std::vector<size_t> next;
      for (size_t j = 1; j < body[i+1].size(); ++j) next.push_back(j);
      size_t m = first[i].size() + next.size()/2;
      body[i].reserve(m);
      size_t j0 = 0;
      size_t j1 = 0;
      while (body[i].size() < m) {
        if (j1 == body[i+1].size()) {
          body[i].emplace_back(first[i][j0], j0, j1);
        } else if (first[i][j0] < body[i+1][j1].value) {
          body[i].emplace_back(first[i][j0], j0, j1);  // ???
        } else {
          // ???
        }
      }
    }
  }

  std::vector<iterator_type> lower_bounds(value_type x) const {
    return {};
  }
};

int main() {
  size_t k = 4;
  std::vector<std::vector<int>> L(k);
  L[0] = {24, 64, 65, 80, 93};
  L[1] = {23, 25, 26};
  L[2] = {13, 44, 62, 66};
  L[3] = {11, 35, 46, 79, 81};

  fractional_cascading<std::vector<std::vector<int>>::iterator> fc(L.begin(), L.end());
  for (auto it: fc.lower_bounds(50))
    printf("%d\n", *it);
}
