#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <utility>
#include <list>
#include <memory>

template <class Tp>
class meldable_heap {
  // based on binomial heap

  struct binomial_tree {
    struct node {
      std::unique_ptr<node> child = nullptr;
      std::unique_ptr<node> sibling = nullptr;
      Tp value;

      Tp operator *() const { return value; }
      node(const Tp& x): value(x) {}
    };

    std::unique_ptr<node> root;
    size_t rank;

    binomial_tree(): root(nullptr), rank(-1) {};
    binomial_tree(const Tp& x): root(new node(x)), rank(0) {};
    binomial_tree(binomial_tree&& other):
      root(std::move(other.root)), rank(other.rank)
    {
      other.root = nullptr;
      other.rank = -1;
    }
    binomial_tree(std::unique_ptr<node>& tmproot, size_t rank):
      root(std::move(tmproot)), rank(rank)
    {
      root->sibling = nullptr;
    }

    binomial_tree& operator =(binomial_tree&& other) {
      root = std::move(other.root);
      rank = other.rank;
      return *this;
    }

    void meld(binomial_tree& other) { meld(std::move(other)); }
    void meld(binomial_tree&& other) {
      assert(rank == other.rank);
      if (**root < **other.root) root.swap(other.root);
      other.root->sibling = std::move(root->child);
      root->child = std::move(other.root);
      ++rank;
      other.root = nullptr;
      other.rank = -1;
    }

    void inspect() const {
      fprintf(stderr, "rank: %zu, top: %d\n", rank, **root);
    }
  };

  using node = typename binomial_tree::node;

  size_t sz = 0;
  std::list<binomial_tree> c;
  typename std::list<binomial_tree>::iterator max;

  void coalesce() {
    std::vector<binomial_tree> t;
    size_t n = 0;
    for (const auto& bt: c)
      n += size_t(1) << bt.rank;

    if (n == 0) {
      max = c.begin();
      assert(sz == 0);
      return;
    }

    n = size_t(1) << (63 - __builtin_clzll(n));
    t.resize(n);

    for (auto& bt: c) {
      while (t[bt.rank].root)
        bt.meld(std::move(t[bt.rank]));
      t[bt.rank] = std::move(bt);
    }

    c.clear();
    for (auto& bt: t) {
      if (bt.root) c.emplace_back(std::move(bt));
    }
    max = c.begin();
    for (auto it = c.begin(); it != c.end(); ++it)
      if (**max->root < **it->root)
        max = it;
  }

public:
  meldable_heap(): max(c.begin()) {}

  meldable_heap(const Tp& x): sz(1) {
    c.emplace_back(x);
    max = c.begin();
  }

  bool empty() const { return c.empty(); }
  size_t size() const { return sz; }

  void push(const Tp& x) { meld(meldable_heap(x)); }
  Tp top() const { return **max->root; }

  void pop() {
    std::list<binomial_tree> sg;
    sg.splice(sg.begin(), c, max);

    --sz;
    std::unique_ptr<node> cur(std::move(max->root->child));
    max->root->child = nullptr;
    max->root->sibling = nullptr;
    size_t r = max->rank;
    delete max->root.release();

    while (cur) {
      std::unique_ptr<node> tmp(std::move(cur->sibling));
      c.emplace_back(cur, --r);
      cur = std::move(tmp);
    }

    coalesce();
  }

  void meld(meldable_heap& other) { meld(std::move(other)); }
  void meld(meldable_heap&& other) {
    if (other.empty()) return;

    if (empty() || **max->root < **other.max->root)
      max = other.max;

    c.splice(c.end(), other.c);
    sz += other.sz;
    other.max = other.c.begin();
    other.sz = 0;
  }

  void add_to_all(const Tp& x);  // need this?

  void inspect() const {
    for (const auto& tb: c) {
      tb.inspect();
    }
    fprintf(stderr, "===\n");
  }

  void force_coalesce() { coalesce(); }

  size_t internal_size() { return c.size(); }
};

#include <random>

meldable_heap<int> hoge(size_t n) {
  std::uniform_int_distribution<int> uid(-1000, 1000);
  std::mt19937 rsk(0315);
  meldable_heap<int> res;
  for (size_t i = 0; i < n; ++i)
    res.push(uid(rsk));

  return res;
}

int main() {
  // meldable_heap<int> mh0(1), mh1(2);
  // mh0.inspect();
  // mh1.inspect();
  // fprintf(stderr, "%d\n", mh0.top());
  // mh0.meld(mh1);
  // mh0.force_coalesce();
  // mh0.inspect();
  // fprintf(stderr, "%d\n", mh0.top());

  meldable_heap<int> mh0 = hoge(20);
  printf("%zu\n", mh0.size());
}
