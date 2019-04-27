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
    inspect();
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
    if (t > rank0(size_)) return -1;

    node* cur = root;
    size_t res = 0;
    while (!((cur->left_size - cur->left_one) < t
             && t <= (cur->left_size - cur->left_one) + (cur->size - popcount(cur->value)))) {

      if ((cur->left_size - cur->left_one) + (cur->size - popcount(cur->value)) < t) {
        t -= (cur->left_size - cur->left_one) + (cur->size - popcount(cur->value));
        res += cur->left_size + cur->size;
        cur = cur->children[1];
      } else {
        cur = cur->children[0];
      }
    }
    t -= cur->left_size - cur->left_one;
    res += cur->left_size;
    for (size_t i = 0; t; ++i) {
      if (!(cur->value >> i & 1)) --t;
      ++res;
    }
    return res;
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
      inspect();
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

template <typename Tp, size_t bitlen = 8 * sizeof(Tp)>
class wavelet_matrix {
public:
  using value_type = Tp;

private:
  std::vector<size_t> zeros;
  size_t n;
  std::array<bit_vector, bitlen> a;

  size_t start_index(value_type x) const {
    size_t s = 0;
    size_t t = 0;
    for (size_t i = bitlen; i-- > 1;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return s;
  }

public:
  template <class ForwardIt>
  wavelet_matrix(ForwardIt first, ForwardIt last): zeros(bitlen) {
    std::vector<value_type> c(first, last);
    n = c.size();
    std::vector<value_type> whole = c;
    for (size_t i = bitlen; i--;) {
      std::vector<value_type> zero, one;
      std::vector<bool> vb(n);
      for (size_t j = 0; j < n; ++j) {
        ((whole[j] >> i & 1)? one:zero).push_back(whole[j]);
        vb[j] = (whole[j] >> i & 1);
      }

      zeros[bitlen-i-1] = zero.size();
      a[bitlen-i-1] = bit_vector(vb);
      if (i == 0) break;
      whole = std::move(zero);
      whole.insert(whole.end(), one.begin(), one.end());
    }
  }

  size_t rank(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }
    }
    return t - s;
  }

  std::pair<bool, value_type> max_lt(value_type x, size_t s, size_t t) const {
    return max_le(x-1, s, t);
  }
  std::pair<bool, value_type> max_le(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 1);
      if (reverted) tg = 0;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 0;

      reverted = false;
      if (tg == 1) {
        if (ok0) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok1) {
          ch = 1;
        } else {
          tight = false;
        }
      } else if (!ok0 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << i) - 1;
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= value_type(1) << i;
      }
    }
    return {true, res};
  }
  std::pair<bool, value_type> min_gt(value_type x, size_t s, size_t t) const {
    return min_ge(x+1, s, t);
  }
  std::pair<bool, value_type> min_ge(value_type x, size_t s, size_t t) const {
    if (s == t) return {false, 0};
    size_t ri = bitlen+1;
    size_t rs = -1;
    size_t rt = -1;
    bool tight = true;
    bool reverted = false;
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      size_t tg = (tight? (x >> i & 1) : 0);
      if (reverted) tg = 1;

      bool ok0 = (z > 0);
      bool ok1 = (z < t-s);
      size_t ch = 1;

      reverted = false;
      if (tg == 0) {
        if (ok1) {
          ri = i;
          rs = s;
          rt = t;
        }
        if (ok0) {
          ch = 0;
        } else {
          tight = false;
        }
      } else if (!ok1 && tight) {
        if (ri > bitlen) return {false, 0};
        i = ri+1;
        s = rs;
        t = rt;
        tight = false;
        value_type mask = (static_cast<value_type>(1) << ri) - 1;  // suspicious
        res |= mask;
        res ^= mask;
        reverted = true;
        continue;
      }

      if (ch == 0) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        res |= static_cast<value_type>(1) << i;
      }
    }
    return {true, res};
  }

  value_type quantile(size_t k, size_t s, size_t t) const {
    value_type res = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t z = a[j].rank0(t) - a[j].rank0(s);
      if (k < z) {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      } else {
        res |= static_cast<value_type>(1) << i;
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        k -= z;
      }
    }
    return res;
  }

  std::array<size_t, 3> rank_three_way(value_type x, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if (x >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
        size_t d = tmp - (t-s);
        eq -= d;
        lt += d;
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
        size_t d = tmp - (t-s);
        eq -= d;
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  std::array<size_t, 3> xored_rank_three_way(value_type x, value_type y, size_t t) const {
    if (t == 0) return {0, 0, 0};

    size_t lt = 0;
    size_t eq = t;
    size_t gt = 0;

    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      size_t tmp = (t - s);
      if ((x ^ y) >> i & 1) {
        s = zeros[j] + a[j].rank1(s);
        t = zeros[j] + a[j].rank1(t);
      } else {
        s = a[j].rank0(s);
        t = a[j].rank0(t);
      }

      size_t d = tmp - (t-s);
      eq -= d;
      if (y >> i & 1) {
        lt += d;
      } else {
        gt += d;
      }
    }
    return {lt, eq, gt};
  }

  size_t select(value_type x, size_t t) const {
    if (t == 0) return 0;
    size_t si = start_index(x);
    t += a[bitlen-1].rank(x & 1, si);
    t = a[bitlen-1].select(x & 1, t);

    for (size_t i = 1; i < bitlen; ++i) {
      size_t j = bitlen-i-1;
      if (x >> i & 1) t -= zeros[j];
      t = a[j].select(x >> i & 1, t);
    }

    return t;
  }

  value_type operator [](size_t t) const {
    value_type res = 0;
    size_t s = 0;
    for (size_t i = bitlen; i--;) {
      size_t j = bitlen-i-1;
      if (a[j][s+t]) {
        t = a[j].rank(1, s+t+1) - 1;
        s = zeros[j];
        res |= static_cast<value_type>(1) << i;
      } else {
        t = a[j].rank(0, s+t+1) - 1;
        s = 0;
      }
    }
    return res;
  }

  void inspect() const {
    for (size_t i = 0; i < bitlen; ++i) {
      fprintf(stderr, "%zu (%zu): ", i, zeros[i]);
      for (size_t j = 0; j < n; ++j)
        fprintf(stderr, "%d%c", a[i][j], j+1<n? ' ':'\n');
    }
  }
};

#include <random>

void test_wm() {
  size_t n = 16384;
  int m = 1024;

  std::mt19937 rsk(0315);
  std::uniform_int_distribution<int> nya(0, m);

  std::vector<int> base(n);
  for (size_t i = 0; i < n; ++i)
    base[i] = nya(rsk);

  // for (size_t i = 0; i < n; ++i)
  //   fprintf(stderr, "%d%c", base[i], i+1<n? ' ':'\n');

  std::vector<int> a{
    11,  0, 15,  6,
     5,  2,  7, 12,
    11,  0, 12, 12,
    13,  4,  6, 13,
     1, 11,  6,  1,
     7, 10,  2,  7,
    14, 11,  1,  7,
     5,  4, 14,  6
  };

  n = a.size();
  m = 16;
  base = a;

  wavelet_matrix<int, 12> wm(base.begin(), base.end());

  for (size_t i = 0; i < n; ++i) {
    int expected = base[i];
    int got = wm[i];
    fprintf(stderr, "expected: %d, got: %d\n", expected, got);
    assert(wm[i] == base[i]);
  }

  for (int j = 0; j < m; ++j) {
    fprintf(stderr, "%d:\n", j);
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
      // fprintf(stderr, "expects: %zu, got: %zu\n", count, wm.rank(j, i));
      assert(count == wm.rank(j, i));
      if (base[i] == j) {
        ++count;
        // fprintf(stderr, "expects: %zu, got: %zu\n", i+1, wm.select(j, count));
        assert(i+1 == wm.select(j, count));
      }
    }
    // fprintf(stderr, "expects: %zu, got: %zu\n", count, wm.rank(j, n));
    assert(count == wm.rank(j, n));
  }
}

int main() {
  test_wm();
}
