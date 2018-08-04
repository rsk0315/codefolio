#include <cstdio>
#include <cassert>
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
#define RELEASE
#ifdef RELEASE
#  define fprintf(...) void(0)
#endif  /* RELEASE */

class EulerTourForest;

template <class T>
class RedBlackTree {
  friend EulerTourForest;
  static constexpr T MAX=std::numeric_limits<T>::max();

  size_t size_=0;
  struct Node {
    Node *children[2]={nullptr, nullptr}, *parent=nullptr;
    T value;
    T mins[2]={MAX, MAX};
    size_t lnum=0;
    enum Color: bool { RED, BLACK } color=RED;
    Node(const T &x): value(x) {}

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

  T submin(Node *subroot) const {
    if (!subroot) return MAX;
    return std::min({subroot->mins[0], subroot->value, subroot->mins[1]});
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

    for (int i=0; i<=1; ++i)
      cur->mins[i] = submin(cur->children[i]);
    for (int i=0; i<=1; ++i)
      child->mins[i] = submin(child->children[i]);
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
    while (cur->parent) {
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
    while (cur) {
      res += cur->lnum+1;
      cur = cur->children[1];
    }
    return res;
  }

  size_t size(Node *cur) const {
    if (cur == root) return size_;
    if (cur == cur->parent->children[0]) return cur->parent->lnum;
    return calc_size(cur);
  }

  void reset_min(Node *cur) {
    T min;
    while (cur->parent) {
      min = std::min({cur->mins[0], cur->value, cur->mins[1]});
      cur->parent->mins[cur == cur->parent->children[1]] = min;
      cur = cur->parent;
    }
  }

  void reset_min() {
    std::function<T (Node *)> dfs=[&](Node *cur) {
      if (!cur) return MAX;

      cur->mins[0] = dfs(cur->children[0]);
      cur->mins[1] = dfs(cur->children[1]);
      return std::min({cur->mins[0], cur->value, cur->mins[1]});
    };
    dfs(root);
  }

  T min(Node *cur, size_t il, size_t ir) const {
    assert(cur);

    if (!cur->children[0] && ir == 0)
      return cur->value;

    T res=MAX;
    size_t lnum=cur->lnum;
    if (il == 0 && ir >= lnum) {
      res = std::min(res, cur->mins[0]);
      il = lnum;
    }
    if (il <= lnum && ir+1 == size(cur)) {
      res = std::min(res, cur->mins[1]);
      ir = lnum;
    }
    if (il == lnum && ir == lnum)
      return std::min(res, cur->value);
    if (ir == lnum)
      return std::min({res, cur->value, min(cur->children[0], il, ir-1)});
    if (ir < lnum)
      return min(cur->children[0], il, ir);
    if (il == lnum)
      return std::min({res, cur->value, min(cur->children[1], 0, ir-lnum-1)});
    if (il > lnum)
      return min(cur->children[1], il-lnum-1, ir-lnum-1);

    assert(il < lnum && lnum < ir);
    res = std::min(res, cur->value);
    return std::min({
        min(cur->children[0], il, lnum-1),
        res,
        min(cur->children[1], 0, ir-lnum-1)});
  }

  void release() {
    root = nullptr;
    size_ = 0;
  }

public:
  RedBlackTree() = default;

  template <class ForwardIt>
  RedBlackTree(ForwardIt first, ForwardIt last) {
    fprintf(stderr, "+ %d\n", *first);
    root = new Node(*first);
    root->color = Node::BLACK;
    Node *back=root;
    for (ForwardIt it=std::next(first); it!=last; ++it) {
      fprintf(stderr, "+ %d\n", *it);
      Node *node=new Node(*it);
      node->color = Node::RED;
      back->children[1] = node;
      node->parent = back;
      insert_fixup(node);
      fprintf(stderr, "*back: %d\n", back->value);
      back = node;
      fprintf(stderr, "*back: %d\n", back->value);
    }
    size_ = last-first;
    reset_min();
    verify();
  }

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
    while (cur->children[0]) {
      cur = cur->children[0];
      ++cur->lnum;
    }
    cur->children[0] = node;
    node->parent = cur;
    node->color = Node::RED;
    cur->mins[0] = node->value;
    reset_min(cur);
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
    if (!root) {
      node->color = Node::BLACK;
      root = node;
      return root;
    }
    Node *cur=last();
    cur->children[1] = node;
    node->parent = cur;
    node->color = Node::RED;
    cur->mins[1] = node->value;
    reset_min(cur);
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
      propagate_lnum(y, -1);
      int pardir=(y == y->parent->children[1]);
      y->parent->children[pardir] = x;
      y->parent->mins[pardir] = submin(x);
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
      y->mins[0] = submin(y->children[0]);
      y->mins[1] = submin(y->children[1]);
      reset_min(y);
    } else if (xparent) {
      for (int i=0; i<=1; ++i)
        xparent->mins[i] = submin(xparent->children[i]);
      reset_min(xparent);
    }

    if (fix_needed) {
      verify();
      erase_fixup(x, xparent);
      verify();
    }

    cur->color = Node::RED;
    cur->children[0] = cur->children[1] = cur->parent = nullptr;
    cur->lnum = 0;
    cur->mins[0] = cur->mins[1] = MAX;
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
      med->mins[0] = med->mins[1] = MAX;
      return root;
    }
    med->color = Node::RED;
    if (root == nullptr) {
      med->lnum = 0;
      med->mins[0] = med->mins[1] = MAX;
      oth.insert_front(med);
      swap(oth);
      return root;
    }
    if (oth.root == nullptr) {
      med->lnum = 0;
      med->mins[0] = med->mins[1] = MAX;
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
    for (int i=0; i<=1; ++i)
      med->mins[i] = submin(med->children[i]);
    reset_min(med);
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

    Node *crit_orig=crit;
    RedBlackTree left(crit->children[0]), right(crit->children[1]);
    crit->children[0] = crit->children[1] = nullptr;

    if (crit == root) {
      release();
      return {left, right};
    }

    int crit_dir=(crit == crit->parent->children[1]);
    crit = crit->parent;

    while (crit) {
      RedBlackTree subtree(crit->children[!crit_dir]);
      crit->children[!crit_dir] = nullptr;
      Node *med=crit;
      bool merge_left=crit_dir;
      if (crit != root) crit_dir = (crit == crit->parent->children[1]);
      crit = crit->parent;

      med->parent = nullptr;
      med->children[0] = med->children[1] = nullptr;
      med->mins[0] = med->mins[1] = MAX;
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
    crit_orig->lnum = 0;
    crit_orig->parent = nullptr;
    crit_orig->mins[0] = crit_orig->mins[1] = MAX;
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

  void node_at(size_t pos, const T &val) {
    Node *node=node_at(pos);
    node->value = val;
    reset_min(node);
  }

  Node *insert(Node *node, Node *dst) {
    node->mins[0] = node->mins[1] = MAX;
    node->color = Node::RED;
    if (dst->children[1]) {
      dst = dst->successor();
      dst->children[0] = node;
    } else {
      dst->children[1] = node;
    }

    ++size_;
    node->parent = dst;
    reset_min(node);
    propagate_lnum(node, 1);
    insert_fixup(node);
    return root;
  }
#if 0
  Node *cshift(size_t il, size_t ir) {
    Node *mr=node_at(ir);
    erase(mr);
    if (il == 0) {
      insert_front(mr);
      return root;
    }

    Node *lm=node_at(il-1);
    insert(mr, lm);
    return root;
  }
#else
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
#endif
  T min(size_t il, size_t ir) const {
    return min(root, il, ir);
  }

  ////////////////////////////////////////////////////////////////////////
  bool verify() const {
#ifdef RELEASE
    return true;
#endif
    if (root == nullptr) {
      fprintf(stderr, "================================================\n");
      fprintf(stderr, "size: 0\n");
      fprintf(stderr, "================================================\n");

      return true;
    }

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
    size_t i=0;
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

      fprintf(stderr, "%zu\t%s%*s%d%s (%zu) [%zu] {%d, %d}%s\n",
              size_-1-i++,
              is_red(subroot)? "\x1b[31;1m":"\x1b[37;1m",
              depth*2+2, "  ", subroot->value,
              "\x1b[0m", bh, subroot->lnum, 
              subroot->mins[0], subroot->mins[1],
              (left && right)? "":" *");

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

int main() {
  size_t n;
  int q;
  scanf("%zu %d", &n, &q);

#if 1
  std::vector<int> a(n);
  for (size_t i=0; i<n; ++i)
    scanf("%d", &a[i]);
  RedBlackTree<int> rb(a.begin(), a.end());
#else 
  RedBlackTree<int> rb;
  for (size_t i=0; i<n; ++i) {
    int a;
    scanf("%d", &a);
    rb.insert_back(a);
  }
  rb.verify();
#endif

  for (int i=0; i<q; ++i) {
    int t;
    scanf("%d", &t);
    if (t == 0) {
      size_t l, r;
      scanf("%zu %zu", &l, &r);
      fprintf(stderr, "Cshift [%zu, %zu]\n", l, r);
      rb.cshift(l, r);
    } else if (t == 1) {
      size_t l, r;
      scanf("%zu %zu", &l, &r);
      fprintf(stderr, "Minimum [%zu, %zu]\n", l, r);
      printf("%d\n", rb.min(l, r));
    } else if (t == 2) {
      size_t pos;
      int val;
      scanf("%zu %d", &pos, &val);
      fprintf(stderr, "Update [%zu] = %d\n", pos, val);
      rb.node_at(pos, val);
    }

    rb.verify();
  }
}
