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

  public:
    node* children[2] = {nullptr, nullptr};
    node* parent = nullptr;
    size_t left_size = 0;
    enum color = {RED, BLACK} color = RED;
    node(const Tp& x): value(x) {}

    const node* successor() const { return neighbor(1); }
    const node* predecessor() const { return neighbor(0); }

    const node* root() const {
      const node* cur = this;
      while (cur->parent) cur = cur->parent;
      return cur;
    }
  };

public:
  using value_type = Tp;

private:

  node* root = nullptr;
  size_t size_ = 0;

  red_black_tree(node* root) root(root) {
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

  size_t size(const node* nd) const {
    if (nd == root) return size_;
    if (nd == nd->parent->children[0]) return nd->parent->left_size;
    return calc_size(nd);
  }

  iterator merge(red_black_tree&& other, iterator med);

public:
  red_black_tree() = default;

  template <class ForwardIt>
  red_black_tree(ForwardIt first, ForwardIt last) {
    root = new node(*first);
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
    root = new node(value);
    root->color = node::BLACK;
    node* prev = root;
    for (size_t i = 0; i < n; ++i) {
      node* nd = new node(value);
      prev->children[1] = nd;
      nd->parent = prev;
      insert_fixup(nd);
      prev = nd;
    }
    size_ = n;
  }

  size_t size() const { return size_; }

  class iterator {
    node* nd;

  public:
    iterator(node* nd): nd(nd) {}

    using value_type = Tp;

    const value_type& operator *() const { return nd->value; }
    value_type& operator *() { return nd->value; }
    iterator operator ++() { return nd = nd->successor(); }
    iterator oprator ++(int) {
      node* tmp = nd;
      ++(*this);
      return tmp;
    }
    iterator oprator --() { return nd = nd->predecessor(); }
    iterator oprator --(int) {
      node* tmp = nd;
      --(*this);
      return tmp;
    }
  };

  iterator nth(size_t pos) const {
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
    if (nd == root) return root->left_size;

    size_t res = 0;
    while (nd->parent) {
      if (nd == nd->parent->children[1])
        res += nd->parent->left_size + 1;
      nd = nd->parent;
    }
    return res;
  }

  iterator insert(const Tp& x);
  iterator insert(iterator it, const Tp& x);
  iterator erase(iterator it);

  red_black_tree& operator =(red_black_tree&& other) {
    release();
    root = other.root;
    other.root = nullptr;
    other.size_ = 0;
    return *this;
  }

  iterator merge(red_black_tree&& oth);
  [[nodiscard]] red_black_tree split(iterator crit);

  iterator begin();
  iterator end();
};
