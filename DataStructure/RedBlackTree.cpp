#include <cstdio>
#include <cassert>
// #define assert(c) (void)({ if (c) void(0); else for (;;) {} })
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <functional>

// for debug
#include <random>
#include <unistd.h>
#include <stdexcept>

// for release
// #define fprintf(...) void(0)

class EulerTourForest;

template <class T>
class RedBlackTree {
  friend EulerTourForest;

  size_t size_=0;
  struct Node {
    Node *children[2]={nullptr, nullptr}, *parent=nullptr;
    T value;
    T lmin=std::numeric_limits<T>::max(), rmin=std::numeric_limits<T>::max();
    size_t lnum=0;
    enum Color: bool { RED, BLACK } color=RED;
    Node(T x): value(x) {}

    Node *successor() {
      Node *cur=this;
      if (children[1]) {
        cur = cur->children[1];
        while (cur->children[0]) cur = cur->children[0];
        return cur;
      }
      while (true) {
        if (cur->parent == nullptr) return nullptr;
        if (cur == cur->parent->children[0]) return cur->parent;
        cur = cur->parent;
      }
    }

    const Node *successor() const {
      const Node *cur=this;
      if (children[1]) {
        cur = cur->children[1];
        while (cur->children[0]) cur = cur->children[0];
        return cur;
      }
      while (true) {
        if (cur->parent == nullptr) return nullptr;
        if (cur == cur->parent->children[0]) return cur->parent;
        cur = cur->parent;
      }
    }

    Node *root() {
      Node *cur=this;
      while (cur->parent != nullptr) cur = cur->parent;
      return cur;
    }

    const Node *root() const {
      const Node *cur=this;
      while (cur->parent != nullptr) cur = cur->parent;
      return cur;
    }

    Node *first() {
      Node *cur=root();
      while (cur->children[0] != nullptr) cur = cur->children[0];
      return cur;
    }

    const Node *first() const {
      const Node *cur=root();
      while (cur->children[0] != nullptr) cur = cur->children[0];
      return cur;
    }
  } *root=nullptr;

  RedBlackTree(Node *root): root(root) {
    if (root) {
      root->parent = nullptr;
      root->color = Node::BLACK;
      reset_size();
    }
  }

  bool is_red(Node *node) const {
    return node && node->color == Node::RED;
  }

  Node *first() {
    if (root == nullptr) return nullptr;
    Node *cur=root;
    while (cur->children[0] != nullptr) cur = cur->children[0];
    return cur;
  }

  const Node *first() const {
    if (root == nullptr) return nullptr;
    const Node *cur=root;
    while (cur->children[0] != nullptr) cur = cur->children[0];
    return cur;
  }

  Node *last() {
    if (root == nullptr) return nullptr;
    Node *cur = root;
    while (cur->children[1] != nullptr) cur = cur->children[1];
    return cur;
  }

  size_t black_height() const {
    size_t res=0;
    for (Node *cur=root; cur; cur=cur->children[0])
      if (cur->color == Node::BLACK) ++res;

    return res;
  }

  void swap(RedBlackTree &oth) {
    std::swap(size_, oth.size_);
    std::swap(root, oth.root);
  }

  void rotate(Node *cur, int dir) {
    // dir == 0: left-rotate
    // dir == 1: right-rotate

    Node *child=cur->children[!dir];
    assert(child);
    cur->children[!dir] = child->children[dir];
    if (child->children[dir])
      child->children[dir]->parent = cur;

    child->parent = cur->parent;
    if (cur->parent == nullptr) {
      root = child;
    } else if (cur == cur->parent->children[dir]) {
      cur->parent->children[dir] = child;
    } else {
      cur->parent->children[!dir] = child;
    }
    child->children[dir] = cur;
    cur->parent = child;

    if (dir == 0) {
      cur->parent->lnum += cur->lnum+1;
    } else if (dir == 1) {
      cur->lnum -= cur->parent->lnum+1;
    }
  }

  Node *successor(Node *cur) const {
    if (cur->children[1]) {
      cur = cur->children[1];
      while (cur->children[0])
        cur = cur->children[0];

      return cur;
    }

    while (true) {
      if (cur->parent == nullptr) return nullptr;
      if (cur == cur->parent->children[0]) return cur->parent;
      cur = cur->parent;
    }
  }

  void insert_fixup(Node *cur) {
    while (is_red(cur->parent)) {
      Node *gparent=cur->parent->parent;
      assert(gparent);

      int uncle_dir=(cur->parent != gparent->children[1]);
      Node *uncle=gparent->children[uncle_dir];

      if (is_red(uncle)) {
        cur->parent->color = uncle->color = Node::BLACK;
        gparent->color = Node::RED;
        cur = gparent;
        continue;
      }
      if (cur == cur->parent->children[uncle_dir]) {
        cur = cur->parent;
        rotate(cur, !uncle_dir);
      }
      cur->parent->color = Node::BLACK;
      cur->parent->parent->color = Node::RED;
      rotate(gparent, uncle_dir);
    }
    root->color = Node::BLACK;
  }

  void erase_fixup(Node *cur, Node *parent) {
    while (cur != root && !is_red(cur)) {
      int sibling_dir=(cur == parent->children[0]);
      Node *sibling=parent->children[sibling_dir];

      if (is_red(sibling)) {
        sibling->color = Node::BLACK;
        parent->color = Node::RED;
        rotate(parent, !sibling_dir);
        sibling = parent->children[sibling_dir];
      }

      if (sibling) {
        if (!is_red(sibling->children[0]) && !is_red(sibling->children[1])) {
          sibling->color = Node::RED;
          cur = parent;
          parent = cur->parent;
          continue;
        }

        if (!is_red(sibling->children[sibling_dir])) {
          if (sibling->children[!sibling_dir])
            sibling->children[!sibling_dir]->color = Node::BLACK;

          sibling->color = Node::RED;
          rotate(sibling, sibling_dir);
          sibling = parent->children[sibling_dir];
          assert(sibling);
        }

        sibling->color = parent->color;
        sibling->children[sibling_dir]->color = Node::BLACK;
      }

      parent->color = Node::BLACK;
      rotate(parent, !sibling_dir);
      cur = root;
      parent = nullptr;
    }
    if (cur) cur->color = Node::BLACK;
  }

  void propagate_lnum(Node *cur, size_t diff) {
    assert(cur);
    while (cur->parent != nullptr) {
      if (cur == cur->parent->children[0]) {
        cur->parent->lnum += diff;
      }
      cur = cur->parent;
    }
  }

  void reset_size() {
    size_ = calc_size(root);
  }

  size_t calc_size(Node *cur) const {
    size_t res=0;
    while (cur != nullptr) {
      res += cur->lnum+1;
      cur = cur->children[1];
    }
    return res;
  }

  void release() {
    root = nullptr;
    size_ = 0;
  }

public:
  RedBlackTree() = default;

  size_t size() const {
    return size_;
  }

  Node *insert_front(Node *node) {
    ++size_;
    node->lnum = 0;
    if (root == nullptr) {
      node->color = Node::BLACK;
      root = node;
      return root;
    }
    Node *cur=root;
    ++root->lnum;
    while (cur->children[0] != nullptr) {
      cur = cur->children[0];
      ++cur->lnum;
    }
    cur->children[0] = node;
    node->parent = cur;
    node->color = Node::RED;
    insert_fixup(node);
    return root;
  }

  Node *insert_front(const T &x) {
    Node *node=new Node(x);
    return insert_front(node);
  }

  Node *insert_back(Node *node) {
    ++size_;
    node->lnum = 0;
    if (root == nullptr) {
      node->color = Node::BLACK;
      root = node;
      return root;
    }
    Node *cur=last();
    cur->children[1] = node;
    node->parent = cur;
    node->color = Node::RED;
    insert_fixup(node);
    return root;
  }

  Node *insert_back(const T &x) {
    Node *node=new Node(x);
    return insert_back(node);
  }

  Node *erase(Node *cur) {
    if (cur == nullptr) return nullptr;

    --size_;
    Node *y=cur;
    if (cur->children[0] && cur->children[1]) y = successor(y);
    assert(y != nullptr);

    Node *x=y->children[0];
    if (x == nullptr) x = y->children[1];

    if (x) x->parent = y->parent;
    if (y->parent == nullptr) {
      root = x;
    } else {
      if (y == y->parent->children[1]) {
        y->parent->children[1] = x;
      } else {
        propagate_lnum(y, -1);
        y->parent->children[0] = x;
      }
      // y->parent->children[y == y->parent->children[1]] = x;
    }
    Node *xparent=y->parent;  // instead of x->parent; in case x is null
    bool fix_needed=(y->color == Node::BLACK);
    if (y != cur) {
      // keep detached node intact; not use cur->value = y->value.
      y->parent = cur->parent;
      y->children[0] = cur->children[0];
      y->children[1] = cur->children[1];
      y->color = cur->color;
      y->lnum = cur->lnum;

      if (y->children[0]) y->children[0]->parent = y;
      if (y->children[1]) y->children[1]->parent = y;
      if (y->parent) {
        y->parent->children[cur == cur->parent->children[1]] = y;
      } else {
        root = y;
      }
      if (xparent == cur) {
        xparent = y;
        if (x) x->parent = xparent;
      }
      propagate_lnum(y, 1);
    }

    if (fix_needed)
      erase_fixup(x, xparent);

    cur->children[0] = cur->children[1] = cur->parent = nullptr;
    cur->lnum = 0;
    return root;
  }

  Node *merge(RedBlackTree &oth) {
    if (oth.root == nullptr) return root;
    if (root == nullptr) {
      swap(oth);
      return root;
    }

    size_t bh1=black_height(), bh2=oth.black_height();
    if (bh1 >= bh2) {
      Node *med=oth.first();
      oth.erase(med);
      merge(oth, med);
    } else {
      Node *med=last();
      erase(med);
      merge(oth, med);
    }
    return root;
  }

  Node *merge(RedBlackTree &oth, Node *med) {
    if (med == nullptr) {
      merge(oth);
      return root;
    }

    if (root == nullptr && oth.root == nullptr) {
      size_ = 1;
      root = med;
      med->color = Node::BLACK;
      med->lnum = 0;
      return root;
    }
    med->color = Node::RED;
    if (root == nullptr) {
      med->lnum = 0;
      oth.insert_front(med);
      swap(oth);
      return root;
    }
    if (oth.root == nullptr) {
      med->lnum = 0;
      insert_back(med);
      return root;
    }

    size_t bh1=black_height(), bh2=oth.black_height();
    Node *cur;
    if (bh1 >= bh2) {
      cur = root;
      while (bh1 >= bh2) {
        if (cur->color == Node::BLACK && bh1-- == bh2) break;
        cur = cur->children[1];
      }
      med->children[0] = cur;
      med->children[1] = oth.root;
      med->parent = cur->parent;

      if (med->parent == nullptr) {
        root = med;
        med->color = Node::BLACK;
      } else {
        med->parent->children[1] = med;
      }
      cur->parent = oth.root->parent = med;
    } else {
      cur = oth.root;
      while (bh1 <= bh2) {
        if (cur->color == Node::BLACK && bh1 == bh2--) break;
        cur = cur->children[0];
      }
      med->children[0] = root;
      med->children[1] = cur;
      med->parent = cur->parent;

      med->parent->children[0] = med;
      cur->parent = root->parent = med;

      root = oth.root;
      propagate_lnum(med, size_+1);
    }

    med->lnum = calc_size(med->children[0]);
    oth.root = nullptr;
    size_ += oth.size_ + 1;
    oth.size_ = 0;
    insert_fixup(med);
    return root;
  }

  std::pair<RedBlackTree, RedBlackTree> split(Node *crit) {
    if (crit == nullptr) {
      RedBlackTree left(*this);
      release();
      return {left, nullptr};
    }

    RedBlackTree left(crit->children[0]), right(crit->children[1]);
    crit->children[0] = crit->children[1] = nullptr;

    if (crit == root) {
      release();
      return {left, right};
    }

    int crit_dir=(crit == crit->parent->children[1]);
    crit = crit->parent;

    while (crit != nullptr) {
      RedBlackTree subtree(crit->children[!crit_dir]);
      crit->children[!crit_dir] = nullptr;
      Node *med=crit;
      bool merge_left=crit_dir;
      if (crit != root) crit_dir = (crit == crit->parent->children[1]);
      crit = crit->parent;

      med->parent = nullptr;
      med->children[0] = med->children[1] = nullptr;
      if (merge_left) {
        subtree.merge(left, med);
        left.swap(subtree);
      } else {
        right.merge(subtree, med);
      }
    }

    release();
    left.reset_size();
    right.reset_size();
    return {left, right};
  }

  Node *node_at(size_t i) {
    Node *res=root;
    while (res->lnum != i) {
      fprintf(stderr, "i: %zu, ", i);
      fprintf(stderr, "lnum: %zu\n", res->lnum);
      if (res->lnum < i) {
        i -= res->lnum+1;
        res = res->children[1];
      } else {
        res = res->children[0];
      }
    }
    return res;
  }

  Node *cshift(size_t il, size_t ir) {
    Node *lm=node_at(il);
    Node *mr=node_at(ir);

    RedBlackTree left, mid, right, tmp;
    std::tie(left, tmp) = split(lm);
    swap(tmp);
    std::tie(mid, right) = split(mr);

    mid.insert_front(lm);
    mid.merge(right);
    left.merge(mid, mr);
    swap(left);

    return root;
  }

  ////////////////////////////////////////////////////////////////////////
  bool verify() const {
    if (root == nullptr) return true;

    auto report=[](const char *s) {
      fprintf(stderr, "%sviolated:%s %s%s%s\n",
              "\x1b[31;1m", "\x1b[0m",
              "\x1b[37;1m", s, "\x1b[0m");
    };

    bool violated=false;
    if (root->color == Node::RED) {
      violated = true;
      report("root node is red");
    }

    bool reached=false;
    size_t bheight=-1;
    std::function<void (Node *, int, size_t)> dfs=[&](
        Node *subroot, int depth, size_t bh) {

      if (subroot->color == Node::RED) {
        // Note: once violated, we continue checking.  We may not assume
        //       the root node is black.
        if (subroot->parent && subroot->parent->color == Node::RED) {
          violated = true;
          report("two consecutive nodes are red");
        }
      } else {
        ++bh;
      }

      Node *left=subroot->children[0], *right=subroot->children[1];

      if (!(left && right)) {
        if (!reached) {
          bheight = bh;
          reached = true;
        } else if (bheight != bh) {
          violated = true;
          report("different number of black nodes in two root-leaf paths");
        }
      }

      if (right) dfs(right, depth+1, bh);

      fprintf(stderr, "%s%*s%d%s (%zu) [%zu]%s\n",
              is_red(subroot)? "\x1b[31;1m":"\x1b[37;1m",
              depth*2+2, "  ", subroot->value,
              "\x1b[0m", bh, subroot->lnum, (left && right)? "":" *");

      if (left) dfs(left, depth+1, bh);
    };

    fprintf(stderr, "================================================\n");
    fprintf(stderr, "size: %zu\n", size_);
    dfs(root, 0, 0);
    fprintf(stderr, "================================================\n");
    return !violated;
  }
  ////////////////////////////////////////////////////////////////////////
};

// XXX 1508 に accept されるにあたって RedBlackTree には以下の手直しが必要
//     - erase() および merge() した際に適切に lmin と rmin を更新する
//     - min() を定義する

// あれ？ これ lmin だけ管理すればよくないですか？
// どっちのが楽かはわかんないですが

int main() {
  size_t n;
  int q;
  scanf("%zu %d", &n, &q);

  RedBlackTree<int> rb;
  for (size_t i=0; i<n; ++i) {
    int a;
    scanf("%d", &a);
    rb.insert_back(a);
  }
  rb.verify();

  for (int i=0; i<q; ++i) {
    int t;
    scanf("%d", &t);
    if (t == 0) {
      size_t l, r;
      scanf("%zu %zu", &l, &r);
      rb.cshift(l, r);
    } else if (t == 1) {
      size_t l, r;
      scanf("%zu %zu", &l, &r);
      //printf("%d\n", rb.min(l, r));
    } else if (t == 2) {
      size_t pos;
      int val;
      scanf("%zu %d", &pos, &val);
      rb.node_at(pos)->value = val;
    }

    rb.verify();
  }
}
