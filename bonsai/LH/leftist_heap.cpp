#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include <algorithm>

template <class Tp, class Compare = std::less<Tp>>
class meldable_heap {
  // based on leftist heap

public:
  using value_type = Tp;

private:
  meldable_heap(const Tp& x): M_root(new M_node(x)) {}

  struct M_node {
    std::unique_ptr<M_node> left{nullptr}, right{nullptr};
    size_t s = 0;
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
    if (!base->left || base->left->s < base->right->s)
      base->left.swap(base->right);
    base->s = (base->right? base->right->s+1 : 1);
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

int main() {
  meldable_heap<int> q;
  q.push(0);
  q.push(4);
  q.push(1);
  meldable_heap<int> r;
  r.push(2);
  r.push(5);
  meldable_heap<int> s = r;
  q.meld(r);

  fprintf(stderr, "q: %zu\n", q.size());
  while (!q.empty()) {
    fprintf(stderr, "%d\n", q.top());
    q.pop();
  }

  fprintf(stderr, "r: %zu\n", r.size());
  while (!r.empty()) {
    fprintf(stderr, "%d\n", r.top());
    r.pop();
  }

  fprintf(stderr, "s: %zu\n", s.size());
  while (!s.empty()) {
    fprintf(stderr, "%d\n", s.top());
    s.pop();
  }
}
