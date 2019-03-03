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
    self_iterator lb_next, ub_next;
    node_type() {}
    node_type(const value_type& value): value(value) {}
    node_type(const value_type& value, iterator_type iter,
              self_iterator lb_next, self_iterator ub_next):
      value(value), iter(iter), lb_next(lb_next), ub_next(ub_next)
    {}

    const value_type& operator *() const { return value; }
    bool operator <(const node_type& other) const { return **this < *other; }
  };

  std::vector<std::vector<node_type>> body;
  std::vector<iterator_type> begins, ends;

public:
  fractional_cascading(BidirIt first, BidirIt last) {
    size_t n = std::distance(first, last);
    body.resize(n+1);
    begins.resize(n);
    ends.resize(n);

    auto body_next = body.end();
    auto body_current = body_next-1;
    auto begins_i = begins.end();
    auto ends_i = ends.end();
    BidirIt it = last;
    do {
      --it;
      --body_next;
      --body_current;
      size_t m = it->size() + body_next->size()/2;
      body_current->reserve(m);
      *--begins_i = it->begin();
      *--ends_i = it->end();
      iterator_type j0 = it->begin();
      typename node_type::self_iterator j1 = body_next->begin();
      auto lb_next = j1;
      auto ub_next = j1;
      if (j1 < body_next->end()) ++j1;
      while (body_current->size() < m) {
        if (j1 == body_next->end() || (j0 != it->end() && *j0 < **j1)) {
          while (lb_next < body_next->end() && **lb_next < *j0) ++lb_next;
          while (ub_next < body_next->end() && !(*j0 < **ub_next)) ++ub_next;
          body_current->emplace_back(*j0, j0, lb_next, ub_next);
          ++j0;
        } else {
          body_current->emplace_back(**j1, j0, j1, j1);
          if (++j1 < body_next->end()) ++j1;  // +2 may cause undefined behavior
        }
      }
    } while (it != first);

    inspect();
  }

  std::vector<iterator_type> lower_bounds(value_type x) const {
    auto it = std::lower_bound(body[0].begin(), body[0].end(), node_type(x));
    std::vector<iterator_type> res{it->iter};
    for (size_t i = 1; i+1 < body.size(); ++i) {
      auto ub = it->lb_next;
      auto lb = ub-1;
      it = (!(**lb < x)? lb:ub);
      res.push_back(it->iter);
    }
    return res;
  }

  std::vector<iterator_type> upper_bounds(value_type x) const {
    auto it = std::upper_bound(body[0].begin(), body[0].end(), node_type(x));
    std::vector<iterator_type> res{it->iter};
    for (size_t i = 1; i+1 < body.size(); ++i) {
      auto ub = it->ub_next;
      auto lb = ub-1;
      it = ((x < **lb)? lb:ub);
      res.push_back(it->iter);
    }
    return res;
  }

  void inspect() const {
    for (size_t i = 0; i+1 < body.size(); ++i)
      for (size_t j = 0; j < body[i].size(); ++j)
        fprintf(stderr, "\x1b[31;1m%d\x1b[m[%td, %td]%c",
                *body[i][j], body[i][j].iter-begins[i], body[i][j].lb_next-body[i+1].begin(),
                j+1<body[i].size()? ' ':'\n');

    fprintf(stderr, "===\n");
    for (size_t i = 0; i+1 < body.size(); ++i)
      for (size_t j = 0; j < body[i].size(); ++j)
        fprintf(stderr, "\x1b[31;1m%d\x1b[m[%td, %td]%c",
                *body[i][j], body[i][j].iter-begins[i], body[i][j].ub_next-body[i+1].begin(),
                j+1<body[i].size()? ' ':'\n');
  }
};

int main() {
  size_t k = 4;
  std::vector<std::vector<int>> L(k);
  const int INF = 1e4;
  L[0] = {24, 64, 65, 80, 93, INF};
  L[1] = {23, 25, 26, INF};
  L[2] = {13, 44, 62, 62, 62, 62, 66, INF};
  L[3] = {11, 35, 46, 79, 81, INF};

  fractional_cascading<std::vector<std::vector<int>>::iterator> fc(L.begin(), L.end());

  int q = 62;

  auto lbs = fc.lower_bounds(q);
  for (size_t i = 0; i < lbs.size(); ++i)
    printf("[%td]: %d\n", lbs[i]-L[i].begin(), *lbs[i]);

  auto ubs = fc.upper_bounds(q);
  for (size_t i = 0; i < ubs.size(); ++i)
    printf("[%td]: %d\n", ubs[i]-L[i].begin(), *ubs[i]);
}
