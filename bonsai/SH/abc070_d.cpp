#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <queue>
#include <utility>
#include <memory>
#include <deque>
#include <functional>

template <class Weight>
struct edge {
  using value_type = Weight;
  size_t src, dst;
  value_type cost;

  edge(size_t src, size_t dst, value_type cost=1):
    src(src), dst(dst), cost(cost)
  {}

  bool operator <(const edge<value_type> &rhs) const {
    if (cost != rhs.cost) return cost < rhs.cost;
    if (src != rhs.src) return src < rhs.src;
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
    template <class... Ts>
    M_node(const Ts&... xs): value(xs...) {}
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
    M_meld(std::unique_ptr<M_node>(new M_node(xs...)));
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

template <class Tp>
constexpr Tp inf = Tp(1) << (8*sizeof(Tp)-3);

template <class Weight>
std::vector<Weight> shortest(const graph<Weight>& g, size_t s) {
  // based on Dijkstra algorithm
  std::vector<Weight> res(g.size(), inf<Weight>);
  meldable_heap<std::pair<Weight, size_t>, std::greater<>> q;
  res[s] = 0;
  q.emplace(0, s);
  while (!q.empty()) {
    Weight w;
    size_t v;
    std::tie(w, v) = q.top();
    q.pop();
    if (w > res[v]) continue;

    for (const auto& e: g[v]) {
      if (res[e.dst] > w + e.cost) {
        res[e.dst] = w + e.cost;
        q.emplace(res[e.dst], e.dst);
      }
    }
  }
  return res;
}

int main() {
  size_t N;
  scanf("%zu", &N);

  graph<intmax_t> g(N);
  for (size_t i = 1; i < N; ++i) {
    size_t a, b;
    intmax_t c;
    scanf("%zu %zu %jd", &a, &b, &c);
    --a;
    --b;
    g.connect_with(a, b, c);
  }

  size_t Q, K;
  scanf("%zu %zu", &Q, &K);
  --K;
  auto dist = shortest(g, K);
  for (size_t i = 0; i < Q; ++i) {
    size_t x, y;
    scanf("%zu %zu", &x, &y);
    --x;
    --y;
    printf("%jd\n", dist[x]+dist[y]);
  }
}
