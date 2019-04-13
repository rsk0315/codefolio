#include <cstdio>
#include <cassert>
#include <algorithm>
#include <vector>

template <class Tp>
class red_black_tree {
  class node {
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
    Tp value;
    size_t left_size = 0;
    enum Color {RED, BLACK} color = RED;
    node(const Tp& x): value(x) {}

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
  using value_type = Tp;

  class iterator {
    friend red_black_tree<Tp>;
    node* nd;

  public:
    iterator(node* nd): nd(nd) {}

    using value_type = Tp;

    const value_type& operator *() const { return nd->value; }
    value_type& operator *() { return nd->value; }
    iterator operator ++() { return nd = nd->successor(); }
    iterator operator ++(int) {
      node* tmp = nd;
      ++(*this);
      return tmp;
    }
    iterator operator --() { return nd = nd->predecessor(); }
    iterator operator --(int) {
      node* tmp = nd;
      --(*this);
      return tmp;
    }

    bool operator !=(const iterator other) const { return nd != other.nd; }
  };

private:
  node* root = nullptr;
  node* first = nullptr;
  size_t size_ = 0;

  red_black_tree(node* root): root(root) {
    if (!root) {
      size_ = 0;
      return;
    }

    root->parent = nullptr;
    root->color = node::BLACK;
    reset_size();
  }

  bool is_red(node* nd) const {
    return nd && (nd->color == node::RED);
  }

  size_t black_height() const {
    size_t res = 0;
    for (node* cur = root; cur; cur = cur->children[0])
      if (cur->color == node::BLACK) ++res;
    return res;
  }

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
      cur->parent->left_size += cur->left_size + 1;
    } else {
      cur->left_size -= cur->parent->left_size + 1;
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

  void reset_size() {
    size_ = calc_size(root);
  }

  size_t calc_size(const node* nd) const {
    size_t res = 0;
    while (nd) {
      res += nd->left_size + 1;
      nd = nd->children[1];
    }
    return res;
  }

  void propagate_left_size(node* cur, size_t diff) {
    while (cur->parent) {
      if (cur == cur->parent->children[0])
        cur->parent->left_size += diff;
      cur = cur->parent;
    }
  }

  node* insert(node* nd, node* before_of) {
    assert(nd);

    nd->children[0] = nd->children[1] = nullptr;
    nd->left_size = 0;
    nd->color = node::RED;

    if (!before_of) {
      before_of = root;
      if (before_of)
        while (before_of->children[1]) before_of = before_of->children[1];
      before_of->children[1] = nd;
    } else if (before_of->children[0]) {
      before_of = before_of->predecessor();
      before_of->children[1] = nd;
    } else {
      before_of->children[0] = nd;
    }
    if (before_of == first) first = nd;
    nd->parent = before_of;
    ++size_;
    propagate_left_size(nd, 1);
    insert_fixup(nd);
    return nd;
  }

  node* erase(node* nd) {
    assert(nd);

    node* after = nd->successor();

    --size_;
    node* y = nd;
    if (nd->children[0] && nd->children[1]) y = y->successor();

    node* x = y->children[0];
    if (!x) x = y->children[1];
    if (x) x->parent = y->parent;
    if (!y->parent) {
      root = x;
    } else {
      propagate_left_size(y, -1);
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
      y->left_size = nd->left_size;

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
    }
    if (fix_needed) erase_fixup(x, xparent);

    nd->color = node::RED;
    nd->children[0] = nd->children[1] = nd->parent = nullptr;
    nd->left_size = 0;

    if (nd == first) first = after;
    return after;
  }

  size_t size(const node* nd) const {
    if (nd == root) return size_;
    if (nd == nd->parent->children[0]) return nd->parent->left_size;
    return calc_size(nd);
  }

  void release() const {
  }

  iterator merge(red_black_tree&& other, const Tp& med);

public:
  red_black_tree() = default;

  template <class ForwardIt>
  red_black_tree(ForwardIt first, ForwardIt last) {
    this->first = root = new node(*first);
    root->color = node::BLACK;
    node* prev = root;
    for (ForwardIt it = first; ++it != last;) {
      node* nd = new node(*it);
      prev->children[1] = nd;
      nd->parent = prev;
      insert_fixup(nd);
      prev = nd;
    }
    size_ = std::distance(first, last);
  }

  red_black_tree(size_t n, Tp value = Tp()) {
    first = root = new node(value);
    root->color = node::BLACK;
    node* prev = root;
    for (size_t i = 1; i < n; ++i) {
      node* nd = new node(value);
      prev->children[1] = nd;
      nd->parent = prev;
      insert_fixup(nd);
      prev = nd;
    }
    size_ = n;
  }

  size_t size() const { return size_; }

  iterator nth(size_t pos) const {
    if (pos == 0) return first;
    node* res = root;
    while (res->left_size != pos) {
      if (res->left_size < pos) {
        pos -= res->left_size + 1;
        res = res->children[1];
      } else {
        res = res->children[0];
      }
    }
    return res;
  }

  size_t node_index(const node* nd) const {
    if (!nd) return size_;
    if (nd == first) return 0;
    if (nd == root) return root->left_size;

    size_t res = 0;
    while (nd->parent) {
      if (nd == nd->parent->children[1])
        res += nd->parent->left_size + 1;
      nd = nd->parent;
    }
    return res;
  }

  void push_back(const Tp& x) { insert(new node(x), nullptr); }
  void push_front(const Tp& x) {
    node* first = root;
    if (first)
      while (first->children[0]) first = first->children[0];
    insert(new node(x), first);
  }
  void insert(iterator it, const Tp& x) { insert(new node(x), it.nd); }
  template <class ForwardIt>
  void insert(iterator it, ForwardIt first, ForwardIt last) {
    while (first != last) {
      insert(new node(*first), it.nd);
      ++first;
    }
  }

  iterator pop_front() {
    if (!root) return nullptr;
    return erase(first);
  }
  iterator pop_back() {
    node* nd = root;
    if (!nd) return nullptr;
    while (nd->children[1]) nd = nd->children[1];
    return erase(nd);
  }
  iterator erase(iterator it) {
    return erase(it.nd);
  }

  red_black_tree& operator =(red_black_tree&& other) {
    release();
    root = other.root;
    other.root = nullptr;
    other.size_ = 0;
    return *this;
  }

  // iterator merge(red_black_tree&& oth) {
  //   iterator med = oth.first;
  //   if (!oth.root) return med;
  //   if (!root) {
  //     *this = oth;
  //     return nullptr;
  //   }
  //   size_t bh1 = black_height();
  //   size_t bh2 = oth.black_height();
  //   if (bh1 >= bh2) {
  //     Tp tmp = std::move(oth.first->value);
  //     oth.pop_front();
  //     merge(oth, tmp);
  //   } else {
  //     node* right = root;
  //     while (right->children[1]) right = right->children[1];
  //     Tp tmp = std::move(right->value);
  //     pop_back();
  //     merge(oth, tmp);
  //   }
  //   return med;
  // }

  // [[nodiscard]] red_black_tree split(iterator crit);

  iterator begin() { return first; }
  iterator end() { return nullptr; }
};

int main() {
  std::vector<int> a{1, 5, 2, 9, 5, 7};
  red_black_tree<int> rbt(a.begin(), a.end());
  for (int x: rbt) printf("%d\n", x);
  puts("---");

  auto it = rbt.nth(3);
  rbt.insert(it, 4);
  for (int x: rbt) printf("%d\n", x);
}
