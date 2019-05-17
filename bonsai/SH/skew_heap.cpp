#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>
#include <list>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <queue>

template <class Tp, class Compare = std::less<Tp>>
class meldable_heap {
  // based on skew heap

public:
  using value_type = Tp;

private:
  meldable_heap(const Tp& x): M_root(new M_node(x)) {}

  struct M_node {
    std::unique_ptr<M_node> left{nullptr}, right{nullptr};
    Tp value;
    M_node(const Tp& x): value(x) {}
    ~M_node() = default;
  };
  Compare M_comp;
  std::unique_ptr<M_node> M_root{nullptr};
  size_t M_size = 0;

  void M_meld(std::unique_ptr<M_node>& base, std::unique_ptr<M_node>& other) {
    if (!other) return;
    if (!base) {
      base = std::move(other);
      return;
    }

    if (!M_comp(other->value, base->value)) base.swap(other);
    M_meld(base->right, other);
    base->left.swap(base->right);
  }

  void M_meld(std::unique_ptr<M_node>&& other) {
    assert(!other->left && !other->right);
    M_meld(M_root, other);
    ++M_size;
  }

  void M_delete(std::unique_ptr<M_node>& root) {
    if (root->left) M_delete(root->left);
    if (root->right) M_delete(root->right);
  }
  void M_delete() {
    M_size = 0;
    if (!M_root) return;
    M_delete(M_root);
  }

  void M_deep_copy(std::unique_ptr<M_node>& dst, const std::unique_ptr<M_node>& src) {
    if (!src) return;
    dst = std::unique_ptr<M_node>(new M_node(src->value));
    if (src->left) M_deep_copy(dst->left, src->left);
    if (src->right) M_deep_copy(dst->right, src->right);
  }

public:
  meldable_heap() = default;

  template <class ForwardIt>
  meldable_heap(ForwardIt first, ForwardIt last) {
    do push(*first++); while (first != last);
  }
  meldable_heap(const meldable_heap& other) {
    M_deep_copy(M_root, other.M_root);
    M_size = other.M_size;
  }
  meldable_heap(meldable_heap&& other) {
    M_root = std::move(other.M_root);
    M_size = other.M_size;
  }

  meldable_heap& operator =(const meldable_heap& other) {
    M_delete();
    M_size = other.M_size;
    M_deep_copy(M_root, other.M_root);
  }

  meldable_heap& operator =(meldable_heap&& other) {
    M_delete();
    M_size = other.M_size;
    M_root = std::move(other.M_root);
  }

  bool empty() const { return (M_size == 0); }
  size_t size() const { return M_size; }
  Tp top() const { return M_root->value; }

  void push(const Tp& x) { M_meld(std::unique_ptr<M_node>(new M_node(x))); }
  template <class... Ts>
  void emplace(const Ts&... xs) {
    meld(std::unique_ptr<M_node>(new M_node(xs...)));
  }

  void pop() {
    std::unique_ptr<M_node> left(M_root->left.release());
    std::unique_ptr<M_node> right(M_root->right.release());
    delete M_root.release();
    --M_size;
    M_root = std::move(left);
    M_meld(M_root, right);
  }

  void meld(meldable_heap& other) { meld(std::move(other)); }
  void meld(meldable_heap&& other) {
    M_meld(M_root, other.M_root);
    M_size += other.M_size;
    other.M_size = 0;
    assert(!other.M_root);
  }

  void clear() { M_delete(); }
  ~meldable_heap() { M_delete(); }
};

template <class Weight>
struct edge {
  using value_type = Weight;
  size_t src, dst;
  value_type cost;

  edge(size_t src, size_t dst, value_type cost=1):
    src(src), dst(dst), cost(cost)
  {}

  edge reversed() const { return edge(dst, src, cost); }

  bool operator <(const edge &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
    return dst < rhs.dst;
  }

  bool operator >(const edge &rhs) const {
    if (cost != rhs.cost) return cost > rhs.cost;
    if (src != rhs.src) return src > rhs.src;
    return dst < rhs.dst;
  }
};

template <class Weight>
struct graph: public std::vector<std::vector<edge<Weight>>> {
  using value_type = Weight;
  graph(size_t n): std::vector<std::vector<edge<value_type>>>(n) {}

  void connect_with(size_t src, size_t dst, value_type cost=1) {
    (*this)[src].emplace_back(src, dst, cost);
    (*this)[dst].emplace_back(dst, src, cost);
  }
};

class union_find {
  mutable std::vector<intmax_t> c;

public:
  union_find(size_t n): c(n, -1) {}

  size_t find(size_t v) const {
    if (c[v] < 0)
      return v;

    return (c[v] = find(c[v]));
  }

  bool unite(size_t u, size_t v) {
    u = find(u);
    v = find(v);
    if (u == v)
      return false;

    if (-c[u] > -c[v])
      std::swap(u, v);

    c[v] += c[u];
    c[u] = v;
    return true;
  }

  bool connected(size_t u, size_t v) const {
    return (find(u) == find(v));
  }

  size_t size() const { return c.size(); }
  size_t size(size_t v) const {
    return -c[find(v)];
  }
};

template <class F>
class fix_point: F {
public:
  explicit constexpr fix_point(F&& f) noexcept: F(std::forward<F>(f)) {}

  template <class... Args>
  constexpr decltype(auto) operator ()(Args&&... args) const {
    return F::operator ()(*this, std::forward<Args>(args)...);
  }
};

template <class F>
static inline constexpr decltype(auto) make_fix_point(F&& f) noexcept {
  return fix_point<F>{std::forward<F>(f)};
}

template <class Tp>
std::vector<std::pair<size_t, size_t>> euler_tour(const graph<Tp>& g, size_t r) {
  size_t ind = 0;
  std::vector<std::pair<size_t, size_t>> res(g.size());
  make_fix_point([&](auto f, size_t v, size_t p) -> void {
    res[v].first = ind++;
    for (const auto& e: g[v])
      if (e.dst != p) f(e.dst, e.src);

    res[v].second = ind++;
  })(r, -1);
  return res;
}

template <class Weight>
meldable_heap<edge<Weight>, std::greater<edge<Weight>>> dfs(
    const graph<Weight>& g, const graph<Weight>& g0,
    const std::vector<std::pair<size_t, size_t>>& et,
    const edge<Weight>& pe, std::map<edge<Weight>, intmax_t>& qs) {

  meldable_heap<edge<Weight>, std::greater<edge<Weight>>> mh;
  size_t p = pe.src;
  size_t v = pe.dst;
  for (const auto& e: g0[v])
    if (e.dst != p) mh.meld(dfs(g, g0, et, e, qs));

  for (const auto& e: g[v])
    if (e.dst != p) mh.push(e);

  if (p+1 == 0) return mh;

  qs[pe] = qs[pe.reversed()] = -1;

  while (!mh.empty()) {
    auto e = mh.top();
    if (et[v].first <= et[e.dst].first && et[e.dst].second <= et[v].second) {
      mh.pop();
    } else {
      qs[pe] = qs[pe.reversed()] = -pe.cost + e.cost;
      break;
    }
  }
  return mh;
}

int main() {
  size_t n, m;
  scanf("%zu %zu", &n, &m);

  graph<intmax_t> g(n);
  std::set<edge<intmax_t>> es;
  std::map<edge<intmax_t>, size_t> dec;
  union_find uf(n);
  for (size_t i = 0; i < m; ++i) {
    size_t a, b;
    intmax_t w;
    scanf("%zu %zu %jd", &a, &b, &w);
    --a;
    --b;
    g.connect_with(a, b, w);
    edge<intmax_t> e(a, b, w);
    es.emplace(e);
    dec[e] = dec[edge<intmax_t>(b, a, w)] = i;
    uf.unite(a, b);
  }

  if (uf.size(0) != n) {
    for (size_t i = 0; i < m; ++i) puts("-1");
    return 0;
  }

  graph<intmax_t> g0(n);
  intmax_t cost = 0;
  std::set<edge<intmax_t>> es0;
  {
    union_find uf(n);
    for (const auto& e: es) {
      if (uf.connected(e.src, e.dst)) continue;
      cost += e.cost;
      g0.connect_with(e.src, e.dst, e.cost);
      es0.insert(e);
      es0.insert(e.reversed());
      uf.unite(e.src, e.dst);
    }
  }

  auto et = euler_tour(g0, 0);

  std::map<edge<intmax_t>, intmax_t> map;
  dfs(g, g0, et, edge<intmax_t>(-1, 0, 0), map);

  std::vector<intmax_t> res(m, -1);

  for (const auto& e: es) {
    size_t i = dec[e];
    if (!es0.count(e)) {
      res[i] = cost;
    } else {
      intmax_t dcost = map[e];
      if (dcost == -1) {
        res[i] = -1;
      } else {
        res[i] = cost + dcost;
      }
    }
  }

  for (size_t i = 0; i < m; ++i)
    printf("%jd\n", res[i]);
}
