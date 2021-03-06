#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>
#include <tuple>

class bit_vector {
  class node {
    using value_type = uintmax_t;
    const node* neighbor(size_t dir) const {
      const node* cur = this;
      if (children[dir]) {
        cur = cur->children[dir];
        while (cur->children[!dir]) cur = cur->children[!dir];
        return cur;
      }
      while (true) {
        if (!cur->parent) return nullptr;
        if (cur == cur->parent->children[!dir]) return cur->parent;
        cur = cur->parent;
      }
    }
    node* neighbor(size_t dir) {
      node* cur = this;
      if (children[dir]) {
        cur = cur->children[dir];
        while (cur->children[!dir]) cur = cur->children[!dir];
        return cur;
      }
      while (true) {
        if (!cur->parent) return nullptr;
        if (cur == cur->parent->children[!dir]) return cur->parent;
        cur = cur->parent;
      }
    }

  public:
    node* children[2] = {nullptr, nullptr};
    node* parent = nullptr;
    value_type value;
    size_t size = 0;
    size_t left_size = 0;
    size_t left_one = 0;
    enum Color {RED, BLACK} color = RED;
    node(const value_type& x, size_t size): value(x), size(size) {}

    node* successor() { return neighbor(1); }
    const node* successor() const { return neighbor(1); }
    node* predecessor() { return neighbor(0); }
    const node* predecessor() const { return neighbor(0); }

    const node* root() const {
      const node* cur = this;
      while (cur->parent) cur = cur->parent;
      return cur;
    }
  };

public:
  using value_type = uintmax_t;

  class iterator {
    friend bit_vector;
    node* nd = nullptr;
    node* root = nullptr;

    iterator(node* nd, node* root): nd(nd), root(root) {}
  public:
    iterator() {}

    using value_type = uintmax_t;

    const value_type& operator *() const { return nd->value; }
    value_type& operator *() { return nd->value; }
    node* operator ->() const { return nd; }
    iterator& operator ++() {
      nd = nd->successor();
      return *this;
    }
    iterator operator ++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    iterator& operator --() {
      if (nd == nullptr) {
        nd = root;
        while (nd->children[1]) nd = nd->children[1];
      } else {
        nd = nd->predecessor();
      }
      return *this;
    }
    iterator operator --(int) {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator ==(const iterator other) const { return nd == other.nd; }
    bool operator !=(const iterator other) const { return nd != other.nd; }

    bool operator [](size_t i) const {
      assert(i < nd->size);
      return (nd->value) >> i & 1;
    }
  };

private:
  node* root = nullptr;
  node* first = nullptr;
  size_t size_ = 0;

  bool is_red(node* nd) const { return nd && (nd->color == node::RED); }
  size_t popcount(value_type x) const { return __builtin_popcountll(x); }

  iterator begin() const { return iterator(first, root); }
  iterator end() const { return iterator(nullptr, root); }

  void rotate(node* cur, size_t dir) {
    node* child = cur->children[!dir];
    cur->children[!dir] = child->children[dir];
    if (child->children[dir])
      child->children[dir]->parent = cur;

    child->parent = cur->parent;
    if (!cur->parent) {
      root = child;
    } else if (cur == cur->parent->children[dir]) {
      cur->parent->children[dir] = child;
    } else {
      cur->parent->children[!dir] = child;
    }
    child->children[dir] = cur;
    cur->parent = child;

    if (dir == 0) {
      cur->parent->left_size += cur->left_size + cur->size;
      cur->parent->left_one += cur->left_one + popcount(cur->value);
    } else {
      cur->left_size -= cur->parent->left_size + cur->parent->size;
      cur->left_one -= cur->parent->left_one + popcount(cur->parent->value);
    }
  }

  void insert_fixup(node* cur) {
    while (is_red(cur->parent)) {
      node* gparent = cur->parent->parent;

      size_t uncle_dir = (cur->parent != gparent->children[1]);
      node* uncle = gparent->children[uncle_dir];

      if (is_red(uncle)) {
        cur->parent->color = uncle->color = node::BLACK;
        gparent->color = node::RED;
        cur = gparent;
        continue;
      }
      if (cur == cur->parent->children[uncle_dir]) {
        cur = cur->parent;
        rotate(cur, !uncle_dir);
      }
      cur->parent->color = node::BLACK;
      cur->parent->parent->color = node::RED;
      rotate(gparent, uncle_dir);
    }
    root->color = node::BLACK;
  }

  void erase_fixup(node* cur, node* parent) {
    while (cur != root && !is_red(cur)) {
      size_t sibling_dir = (cur == parent->children[0]);
      node* sibling = parent->children[sibling_dir];

      if (is_red(sibling)) {
        sibling->color = node::BLACK;
        parent->color = node::RED;
        rotate(parent, !sibling_dir);
        sibling = parent->children[sibling_dir];
      }

      if (sibling) {
        if (!is_red(sibling->children[0]) && !is_red(sibling->children[1])) {
          sibling->color = node::RED;
          cur = parent;
          parent = cur->parent;
          continue;
        }

        if (!is_red(sibling->children[sibling_dir])) {
          if (sibling->children[!sibling_dir])
            sibling->children[!sibling_dir]->color = node::BLACK;

          sibling->color = node::RED;
          rotate(sibling, sibling_dir);
          sibling = parent->children[sibling_dir];
        }

        sibling->color = parent->color;
        sibling->children[sibling_dir]->color = node::BLACK;
      }

      parent->color = node::BLACK;
      rotate(parent, !sibling_dir);
      cur = root;
      parent = nullptr;
    }
    if (cur) cur->color = node::BLACK;
  }

  void propagate_left_size(node* cur, size_t diff) {
    while (cur->parent) {
      if (cur == cur->parent->children[0])
        cur->parent->left_size += diff;
      cur = cur->parent;
    }
  }

  void propagate_left_one(node* cur, size_t diff) {
    while (cur->parent) {
      if (cur == cur->parent->children[0])
        cur->parent->left_one += diff;
      cur = cur->parent;
    }
  }

  node* insert(node* before_of, node* nd) {
    assert(nd);

    nd->children[0] = nd->children[1] = nullptr;
    nd->left_size = 0;
    nd->left_one = 0;
    nd->color = node::RED;

    if (!before_of) {
      before_of = root;
      if (!before_of) {
        // empty
        nd->color = node::BLACK;
        nd->parent = nullptr;
        size_ += nd->size;
        root = first = nd;
        return nd;
      }
      while (before_of->children[1]) before_of = before_of->children[1];
      before_of->children[1] = nd;
    } else if (before_of->children[0]) {
      before_of = before_of->predecessor();
      before_of->children[1] = nd;
    } else {
      before_of->children[0] = nd;
      if (before_of == first) first = nd;
    }

    nd->parent = before_of;
    size_ += nd->size;
    propagate_left_size(nd, nd->size);
    propagate_left_one(nd, popcount(nd->value));
    insert_fixup(nd);
    return nd;
  }

  node* erase(iterator it) { return erase(it.nd); }
  node* erase(node* nd) {
    assert(nd);

    node* after = nd->successor();

    size_ -= nd->size;
    node* y = nd;
    if (nd->children[0] && nd->children[1]) y = y->successor();

    size_t left_size = nd->left_size;
    size_t left_one = nd->left_one;

    node* x = y->children[0];
    if (!x) x = y->children[1];
    if (x) x->parent = y->parent;
    if (!y->parent) {
      root = x;
    } else {
      propagate_left_size(y, -y->size);  // CHECKME
      propagate_left_one(y, -popcount(y->value));  // CHECKME
      size_t ydir = (y == y->parent->children[1]);
      y->parent->children[ydir] = x;
    }
    node* xparent = y->parent;  // x may be nil
    bool fix_needed = (y->color == node::BLACK);
    if (y != nd) {
      y->parent = nd->parent;
      y->children[0] = nd->children[0];
      y->children[1] = nd->children[1];
      y->color = nd->color;

      if (y->children[0]) y->children[0]->parent = y;
      if (y->children[1]) y->children[1]->parent = y;
      if (y->parent) {
        y->parent->children[nd == nd->parent->children[1]] = y;
      } else {
        root = y;
      }
      if (xparent == nd) {
        xparent = y;
        if (x) x->parent = y;
      }

      propagate_left_size(y, y->size-nd->size);
      y->left_size = left_size;
      propagate_left_one(y, popcount(y->value)-popcount(nd->value));
      y->left_one = left_one;
    }
    // inspect();
    if (fix_needed) erase_fixup(x, xparent);

    nd->color = node::RED;
    nd->children[0] = nd->children[1] = nd->parent = nullptr;
    nd->left_size = 0;
    nd->left_one = 0;

    if (nd == first) first = after;
    delete nd;
    return after;
  }

  std::pair<iterator, size_t> nth(size_t pos) const {
    if (pos == 0) return {begin(), 0};
    node* res = root;
    if (pos == size_) {
      while (res->children[1]) res = res->children[1];
      return {iterator(res, root), res->size-1};
    }
    while (!(res->left_size <= pos && pos < res->left_size + res->size)) {
      if (res->left_size + res->size <= pos) {
        pos -= res->left_size + res->size;
        res = res->children[1];
      } else {
        res = res->children[0];
      }
    }
    pos -= res->left_size;
    return {iterator(res, root), pos};
  }

  size_t rank0(size_t t) const { return t - rank1(t); }
  size_t rank1(size_t t) const {
    if (t == 0) return 0;

    node* cur = root;
    size_t res = 0;
    if (t == size_) {
      while (cur) {
        res += cur->left_one + popcount(cur->value);
        cur = cur->children[1];
      }
      return res;
    }

    while (!(cur->left_size <= t && t < cur->left_size + cur->size)) {
      if (cur->left_size + cur->size <= t) {
        t -= cur->left_size + cur->size;
        res += cur->left_one + popcount(cur->value);
        cur = cur->children[1];
      } else {
        cur = cur->children[0];
      }
    }
    t -= cur->left_size;
    res += cur->left_one;
    res += popcount(cur->value & ((static_cast<value_type>(1) << t) - 1));
    return res;
  }

  size_t select0(size_t t) const {
    if (t == 0) return 0;
    if (t > rank1(size_)) return -1;
    return -1;
  }
  size_t select1(size_t t) const {
    if (t == 0) return 0;
    if (t > rank1(size_)) return -1;

    node* cur = root;
    size_t res = 0;
    while (!(cur->left_one < t && t <= cur->left_one + popcount(cur->value))) {
      if (cur->left_one + popcount(cur->value) < t) {
        t -= cur->left_one + popcount(cur->value);
        res += cur->left_size + cur->size;
        cur = cur->children[1];
      } else {
        cur = cur->children[0];
      }
    }
    t -= cur->left_one;
    res += cur->left_size;
    for (size_t i = 0; t; ++i) {
      if (cur->value >> i & 1) --t;
      ++res;
    }
    return res;
  }

public:
  bit_vector() = default;

  bit_vector(const std::vector<bool>& bv) {
    for (size_t i = 0; i+63 < bv.size(); i += 64) {
      value_type cur = 0;
      for (size_t j = 0; j < 64; ++j)
        if (bv[i+j]) cur |= static_cast<value_type>(1) << j;

      node* newnode = new node(cur, 64);
      insert(nullptr, newnode);
    }

    size_t rem = bv.size() % 64;
    if (rem > 0) {
      size_t i = bv.size() / 64 * 64;
      value_type cur = 0;
      for (size_t j = 0; j < rem; ++j)
        if (bv[i+j]) cur |= static_cast<value_type>(1) << j;

      node* newnode = new node(cur, rem);
      insert(nullptr, newnode);
    }
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  size_t rank(int x, size_t t) const { return x? rank1(t) : rank0(t); }
  size_t select(int x, size_t t) const { return x? select1(t) : select0(t); }

  void insert(size_t t, int x) {
    if (!root) {
      assert(t == 0);
      insert(nullptr, new node((x == 1), 1));
      return;
    }

    iterator it;
    size_t ind;
    if (t == size_) {
      node* cur = root;
      while (cur->children[1]) cur = cur->children[1];
      it = iterator(cur, root);
      ind = cur->size;
    } else {
      std::tie(it, ind) = nth(t);
    }
    bool appended = false;
    if (it->size == 64) {
      value_type tmp = *it;
      value_type hi = tmp >> 32;
      value_type lo = tmp & ((static_cast<value_type>(1) << 32) - 1);
      it->size = 32;
      size_ -= 32;
      *it = hi;
      propagate_left_size(it.nd, -32);
      propagate_left_one(it.nd, -popcount(lo));
      node* newnode = new node(lo, 32);
      insert(it.nd, newnode);
      appended = true;

      if (ind < 32) {
        --it;
      } else {
        ind -= 32;
      }
    }

    value_type tmp = *it;
    value_type hi = tmp >> ind << (ind+1);
    value_type lo = tmp & ((static_cast<value_type>(1) << ind) - 1);
    value_type cur = (hi | lo);
    if (x) {
      cur |= static_cast<value_type>(1) << ind;
      propagate_left_one(it.nd, 1);
    }
    *it = cur;
    ++it->size;
    ++size_;
    propagate_left_size(it.nd, 1);
    if (appended) {
      // inspect();
    }
  }

  void erase(size_t t) {
    iterator it;
    size_t ind;
    std::tie(it, ind) = nth(t);
    if (it->size == 1) {
      erase(it);
      return;
    }

    value_type tmp = *it;
    value_type hi = (ind < 63)? (tmp >> (ind+1) << ind) : 0;
    value_type lo = tmp & ((static_cast<value_type>(1) << ind) - 1);

    value_type cur = (hi | lo);
    if (*it >> ind & 1) {
      propagate_left_one(it.nd, -1);
    }
    *it = cur;
    --it->size;
    --size_;
    propagate_left_size(it.nd, -1);
    // return;

    if (it->successor() && it->size + it->successor()->size <= 64) {
      iterator succ(it->successor(), root);
      *it |= (*succ << (it->size));
      it->size += succ->size;
      size_ += succ->size;
      propagate_left_size(it.nd, succ->size);
      propagate_left_one(it.nd, popcount(*succ));
      erase(succ);
    } else if (it->predecessor() && it->size + it->predecessor()->size <= 64) {
      iterator pred(it->predecessor(), root);
      *pred |= (*it << (pred->size));
      pred->size += it->size;
      size_ += it->size;
      propagate_left_size(pred.nd, it->size);
      propagate_left_one(pred.nd, popcount(*it));
      erase(it);
    }
  }

  bool operator [](size_t t) const {
    iterator it;
    size_t ind;
    std::tie(it, ind) = nth(t);
    return it[ind];
  }

  void inspect() const {
    for (iterator it(first, root); it.nd != nullptr; ++it) {
      fprintf(stderr,
              "%016jx (size: %zu, left size: %zu, left one: %zu)",
              *it, it->size, it->left_size, it->left_one);

      if (it->children[0])
        fprintf(stderr, " L: %zu", it->children[0]->size);

      if (it->children[1])
        fprintf(stderr, " R: %zu", it->children[1]->size);

      fprintf(stderr, "\n");
    }
  }
};

#include <random>

std::mt19937 rsk(0315);
std::uniform_int_distribution<int> nya(0, 1);

int test1() {
  size_t n = 64+64+30;
  n = 64+30;
  std::vector<bool> base(n);
  for (size_t i = 0; i < n; ++i)
    base[i] = nya(rsk);

  bit_vector bv(base);

  fprintf(stderr, "b0:");
  for (size_t i = 0; i < n; ++i) {
    fprintf(stderr, " %d", !!base[i]);
    if (i % 32 == 31 && i+1 != n) fprintf(stderr, "\n  :");
  }
  puts("");

  size_t k = 30;
  fprintf(stderr, "size: %zu\n", bv.size());
  bv.inspect();
  bv.insert(k, 1);

  fprintf(stderr, "b1:");
  for (size_t i = 0; i <= n; ++i) {
    if (i == k) fprintf(stderr, "\x1b[1;91m");
    fprintf(stderr, " %d", !!bv[i]);
    if (i == k) fprintf(stderr, "\x1b[0m");
    if (i % 32 == 31 && i+1 != n) fprintf(stderr, "\n  :");
  }
  puts("");
  bv.inspect();
  ++n;
  fprintf(stderr, "size: %zu\n", bv.size());

  bv.erase(k);
  fprintf(stderr, "b2:");
  for (size_t i = 0; i < n; ++i) {
    fprintf(stderr, " %d", !!bv[i]);
    if (i % 32 == 31 && i+1 != n) fprintf(stderr, "\n  :");
  }
  puts("");
  bv.inspect();
  --n;
  fprintf(stderr, "size: %zu\n", bv.size());

  size_t acc = 0;
  for (size_t i = 1; i <= n; ++i) {
    if (bv[i-1]) ++acc;
    size_t rank = bv.rank(1, i);
    fprintf(stderr, "rank1(%zu): %zu (%zu)\n", i, rank, acc);
    assert(rank == acc);
  }
  std::vector<size_t> selects(n+1, -1);
  for (size_t i = 0, j = 0; i < n; ++i) {
    if (bv[i]) selects[++j] = i+1;
  }
  for (size_t i = 1; i <= n; ++i) {
    size_t select = bv.select(1, i);
    if (select != selects[i]) fprintf(stderr, "\x1b[1;91m");
    fprintf(stderr, "select1(%zu): %zu (%zu)\n", i, select, selects[i]);
    if (select != selects[i]) fprintf(stderr, "\x1b[0m");
    assert(select == selects[i]);
  }

  return 0;
}

std::vector<size_t> naive_rank(const bit_vector& v) {
  size_t n = v.size();
  std::vector<size_t> rank(n+1);
  for (size_t i = 1; i <= n; ++i) {
    rank[i] = rank[i-1];
    if (v[i-1]) ++rank[i];
  }
  return rank;
}

std::vector<size_t> naive_select(const bit_vector& v) {
  size_t n = v.size();
  std::vector<size_t> select(n+1, -1);
  select[0] = 0;
  for (size_t i = 0, j = 0; i < n; ++i)
    if (v[i]) select[++j] = i+1;

  return select;
}

void out(const bit_vector& v, size_t k, size_t h) {
  size_t n = v.size();
  for (size_t i = 0; i < n; ++i) {
    if (i == h) fprintf(stderr, "\x1b[1;91m");
    fprintf(stderr, " %d", !!v[i]);
    if (i == h) fprintf(stderr, "\x1b[0m");
    if (i % k == k-1 || i+1 == n) fprintf(stderr, "\n");
  }
}

void naive_check(const bit_vector& bv) {
  size_t n = bv.size();

  auto nr = naive_rank(bv);
  auto ns = naive_select(bv);

  for (size_t i = 0; i <= n; ++i) {
    size_t r = bv.rank(1, i);
    size_t s = bv.select(1, i);
    if (r != nr[i]) {
      fprintf(stderr, "rank(%zu): expected: %zu, got: \x1b[1;91m%zu\x1b[0m\n",
              i, nr[i], r);
      assert(r == nr[i]);
    }
    if (s != ns[i]) {
      fprintf(stderr, "select(%zu): expected: %zu, got: \x1b[1;91m%zu\x1b[0m\n",
              i, ns[i], s);
      assert(s == ns[i]);
    }
  }
}

void naive_insert_check(const bit_vector& v, const std::vector<bool>& p, size_t t, bool x) {
  for (size_t i = 0; i < t; ++i)
    assert(v[i] == p[i]);

  assert(v[t] == x);
  for (size_t i = t; i < p.size(); ++i)
    assert(v[i+1] == p[i]);

  out(v, 32, t);
}

void naive_erase_check(const bit_vector& v, const std::vector<bool>& p, size_t t) {
  for (size_t i = 0; i < t; ++i)
    assert(v[i] == p[i]);

  for (size_t i = t+1; i < p.size(); ++i)
    assert(v[i-1] == p[i]);

  out(v, 32, t);
}

void test2() {
  size_t n = 1024;

  std::vector<bool> base(n);
  std::mt19937 rsk(0315);
  std::uniform_int_distribution<int> nya(0, 1);
  for (size_t i = 0; i < n; ++i)
    base[i] = nya(rsk);

  bit_vector bv(base);

  size_t times = 100000;
  while (times--) {
    // fprintf(stderr, "loop: %zu\n", ++times);
    bool do_insert = (nya(rsk) || bv.empty());

    size_t m = bv.size();
    // fprintf(stderr, "size: %zu\n", m);

    assert(m == n);
    std::vector<bool> prev(m);
    for (size_t i = 0; i < m; ++i) prev[i] = bv[i];

    if (do_insert) {
      std::uniform_int_distribution<size_t> meow(0, m);
      size_t t = meow(rsk);
      bool x = nya(rsk);
      // fprintf(stderr, "insert(%zu, %d)\n", t, !!x);
      bv.insert(t, x);
      // bv.inspect();
      // naive_insert_check(bv, prev, t, x);
      ++n;
    } else /* do erase */ {
      std::uniform_int_distribution<size_t> meow(0, m-1);
      size_t t = meow(rsk);
      // fprintf(stderr, "erase(%zu)\n", t);
      bv.erase(t);
      // bv.inspect();
      // naive_erase_check(bv, prev, t);
      --n;
    }
    // naive_check(bv);
  }
}

int main() {
  test2();
}
