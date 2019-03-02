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
    iterator_type iter;
    using self_iterator = typename std::vector<node_type>::iterator;
    self_iterator next;
    node_type() {}
    node_type(const value_type& value): value(value) {}
    node_type(const value_type& value, iterator_type iter, self_iterator next):
      value(value), iter(iter), next(next)
    {}
    const value_type& operator *() const { return value; }
    bool operator <(const node_type& other) const { return **this < *other; }
  };

  std::vector<std::vector<node_type>> lbs, ubs;
  std::vector<iterator_type> begins, ends;

public:
  fractional_cascading(BidirIt first, BidirIt last) {
    size_t n = std::distance(first, last);
    lbs.resize(n+1);
    ubs.resize(n+1);
    begins.resize(n);
    ends.resize(n);

    auto lbs_next = lbs.end();
    auto lbs_current = lbs_next-1;
    auto begins_i = begins.end();
    auto ends_i = ends.end();
    BidirIt it = last;
    do {
      --it;
      --lbs_next;
      --lbs_current;
      size_t m = it->size() + lbs_next->size()/2;
      lbs_current->reserve(m);
      *--begins_i = it->begin();
      *--ends_i = it->end();
      iterator_type j0 = it->begin();
      typename node_type::self_iterator j1 = lbs_next->begin();
      auto j1sub = j1;
      if (j1 < lbs_next->end()) ++j1;
      while (lbs_current->size() < m) {
        if (j1 == lbs_next->end() || (j0 != it->end() && *j0 < **j1)) {
          while (j1sub < lbs_next->end() && **j1sub < *j0) ++j1sub;
          lbs_current->emplace_back(*j0, j0, j1sub);
          ++j0;
        } else {
          lbs_current->emplace_back(**j1, j0, j1);
          if (++j1 < lbs_next->end()) ++j1;  // +2 may cause undefined behavior
        }
      }
    } while (it != first);

    auto ubs_next = ubs.end();
    auto ubs_current = ubs_next-1;
    begins_i = begins.end();
    ends_i = ends.end();
    it = last;
    do {
      --it;
      --ubs_next;
      --ubs_current;
      size_t m = it->size() + ubs_next->size()/2;
      ubs_current->reserve(m);
      --begins_i;
      --ends_i;
      iterator_type j0 = it->begin();
      typename node_type::self_iterator j1 = ubs_next->begin();
      auto j1sub = j1;
      if (j1 < ubs_next->end()) ++j1;
      while (ubs_current->size() < m) {
        if (j1 == ubs_next->end() || (j0 != it->end() && *j0 < **j1)) {
          while (j1sub < ubs_next->end() && **j1sub < *j0) ++j1sub;
          ubs_current->emplace_back(*j0, j0, j1sub);
          ++j0;
        } else {
          ubs_current->emplace_back(**j1, j0, j1);  // XXX
          if (++j1 < ubs_next->end()) ++j1;
        }
      }
    } while (it != first);

    inspect();
  }

  std::vector<iterator_type> lower_bounds(value_type x) const {
    auto it = std::lower_bound(lbs[0].begin(), lbs[0].end(), node_type(x));
    std::vector<iterator_type> res{it->iter};

    for (size_t i = 1; i+1 < lbs.size(); ++i) {
      auto ub = it->next;
      auto lb = ub-1;
      it = (!(**lb < x)? lb:ub);
      res.push_back(it->iter);
    }
      
    return res;
  }

  void inspect() const {
    for (size_t i = 0; i+1 < lbs.size(); ++i)
      for (size_t j = 0; j < lbs[i].size(); ++j)
        fprintf(stderr, "\x1b[31;1m%d\x1b[m[%td, %td]%c",
                *lbs[i][j], lbs[i][j].iter-begins[i], lbs[i][j].next-lbs[i+1].begin(),
                j+1<lbs[i].size()? ' ':'\n');

    fprintf(stderr, "===\n");
    for (size_t i = 0; i+1 < ubs.size(); ++i)
      for (size_t j = 0; j < ubs[i].size(); ++j)
        fprintf(stderr, "\x1b[31;1m%d\x1b[m[%td, %td]%c",
                *ubs[i][j], ubs[i][j].iter-begins[i], ubs[i][j].next-ubs[i+1].begin(),
                j+1<ubs[i].size()? ' ':'\n');
  }
};

int main() {
  size_t k = 4;
  std::vector<std::vector<int>> L(k);
  const int INF = 1e4;
  L[0] = {24, 64, 65, 80, 93, INF};
  L[1] = {23, 25, 26, INF};
  L[2] = {13, 44, 62, 62, 62, 62, 62, 66, INF};
  L[3] = {11, 35, 46, 79, 81, INF};

  fractional_cascading<std::vector<std::vector<int>>::iterator> fc(L.begin(), L.end());

  auto lbs = fc.lower_bounds(62);
  for (size_t i = 0; i < lbs.size(); ++i)
    printf("[%td]: %d\n", lbs[i]-L[i].begin(), *lbs[i]);
}
