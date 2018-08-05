#include <cstdio>
#include <cassert>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <limits>

// #define RELEASE
#ifdef RELEASE
#  define fprintf(...) void(0)
#endif  /* RELEASE */

template <class T>
class RedBlackTree {
  static constexpr T MAX=std::numeric_limits<T>::max();

  size_t size_=0;
  struct Node {
    Node *children[2]={nullptr, nullptr}, *parent=nullptr;
    T value, mins[2]={MAX, MAX};
    size_t lnum=0;
    enum Color { RED, BLACK } color=RED;
    Node(const T &x): value(x) {}

    const Node *successor() const {
      const Node *cur=this;
      if (children[1]) {
        cur = cur->children[1];
        while (cur->children[0]) cur = cur->children[0];
        return cur;
      }
      while (true) {
        if (!cur->parent) return nullptr;
        if (cur == cur->parent->children[0]) return cur->parent;
        cur = cur->parent;
      }
    }

    Node *successor() {
      Node *cur=this;
      if (children[1]) {
        cur = cur->children[1];
        while (cur->children[0]) cur = cur->children[0];
        return cur;
      }
      while (true) {
        if (!cur->parent) return nullptr;
        if (cur == cur->parent->children[0]) return cur->parent;
        cur = cur->parent;
      }
    }

    const Node *root() const {
      const Node *cur=this;
      while (cur->parent) cur = cur->parent;
      return cur;
    }

    const Node *first() const {
      const Node *cur=root();
      while (cur->children[0]) cur = cur->children[0];
      return cur;
    }

    const Node *last() const {
      const Node *cur=root();
      while (cur->children[1]) cur = cur->children[1];
      return cur;
    }
  } *root=nullptr;

  RedBlackTree(Node *root): root(root) {
    if (!root) {
      size_ = 0;
      return;
    }
    root->parent = nullptr;
    root->color = Node::BLACK;
    reset_size();
  }

  bool is_red(Node *node) const {
    return node && node->color == Node::RED;
  }

  size_t black_height() const {
    size_t res=0;
    for (Node *cur=root; cur; cur=cur->children[0])
      if (cur->color == Node::BLACK) ++res;

    return res;
  }

  const Node *first() const {
    if (!root) return nullptr;
    const Node *cur=root;
    while (cur->children[0]) cur = cur->children[0];
    return cur;
  }

  Node *first() {
    if (!root) return nullptr;
    Node *cur=root;
    while (cur->children[0]) cur = cur->children[0];
    return cur;
  }

  const Node *last() const {
    if (!root) return nullptr;
    const Node *cur=root;
    while (cur->children[1]) cur = cur->children[1];
    return cur;
  }

  Node *last() {
    if (!root) return nullptr;
    Node *cur=root;
    while (cur->children[1]) cur = cur->children[1];
    return cur;
  }

  const Node *successor(const Node *cur) const {
    return cur->successor();
  }

  Node *successor(Node *cur) {
    return cur->successor();
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
    // 0/1: left/right rotate

    Node *child=cur->children[!dir];
    assert(child);
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
      cur->parent->lnum += cur->lnum+1;
    } else if (dir == 1) {
      cur->lnum -= cur->parent->lnum+1;
    } else {
      assert(false);
    }

    for (int i=0; i<=1; ++i)
      cur->mins[i] = submin(cur->children[i]);
    for (int i=0; i<=1; ++i)
      child->mins[i] = submin(child->children[i]);
  }

  void insert_fixup(Node *cur) {
    // cur is the newly-inserted node

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
    // cur is `x'
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
    // cur keeps intact
    assert(cur);
    while (cur->parent) {
      if (cur == cur->parent->children[0])
        cur->parent->lnum += diff;
      cur = cur->parent;
    }
  }

  void reset_size() {
    size_ = calc_size(root);
  }

  size_t calc_size(const Node *subroot) const {
    size_t res=0;
    while (subroot) {
      res += subroot->lnum+1;
      subroot = subroot->children[1];
    }
    return res;
  }

  size_t size(const Node *subroot) const {
    if (subroot == root) return size_;
    if (subroot == subroot->parent->children[0])
      return subroot->parent->lnum;
    return calc_size(subroot);
  }

  void reset_min(Node *cur) {
    // cur keeps intact
    assert(cur);
    T min;
    while (cur->parent) {
      min = std::min({cur->mins[0], cur->value, cur->mins[1]});
      cur->parent->mins[cur == cur->parent->children[1]] = min;
      cur = cur->parent;
    }
  }

  void reset_min() {
    // resets all mins
    std::function<T (Node *)> dfs=[&](Node *subroot) {
      if (!subroot) return MAX;

      subroot->mins[0] = dfs(subroot->children[0]);
      subroot->mins[1] = dfs(subroot->children[1]);
      return std::min({subroot->mins[0], subroot->value, subroot->mins[1]});
    };
    dfs(root);
  }

  T min(const Node *subroot, size_t il, size_t ir) const {
    // minimum value over [il, ir] (inclusive)
    assert(subroot);

    if (!subroot->children[0] && ir == 0)
      return subroot->value;

    T res=MAX;
    size_t lnum=subroot->lnum;
    if (il == 0 && ir >= lnum) {
      res = std::min(res, subroot->mins[0]);
      il = lnum;
    }
    if (il <= lnum && ir+1 == size(subroot)) {
      res = std::min(res, subroot->mins[1]);
      ir = lnum;
    }
    if (il == lnum && ir == lnum)
      return std::min(res, subroot->value);
    if (ir == lnum)
      return std::min({
          res, subroot->value, min(subroot->children[0], il, ir-1)});
    if (ir < lnum)
      return min(subroot->children[0], il, ir);
    if (il == lnum)
      return std::min({
          res, subroot->value, min(subroot->children[1], 0, ir-lnum-1)});
    if (il > lnum)
      return min(subroot->children[1], il-lnum-1, ir-lnum-1);

    assert(il < lnum && lnum < ir);
    res = std::min(res, subroot->value);
    return std::min({
        min(subroot->children[0], il, lnum-1), res,
        min(subroot->children[1], 0, ir-lnum-1)});
  }

  void release() {
    root = nullptr;
    size_ = 0;
  }

  Node *node_at(size_t i) {
    Node *res=root;
    while (res->lnum != i) {
      if (res->lnum < i) {
        i -= res->lnum+1;
        res = res->children[1];
      } else {
        res = res->children[0];
      }
    }
    return res;
  }

public:
  RedBlackTree() = default;

  template <class ForwardIt>
  RedBlackTree(ForwardIt first, ForwardIt last) {
    root = new Node(*first);
    root->color = Node::BLACK;
    Node *prev=root;
    for (ForwardIt it=first; ++it!=last;) {
      Node *node=new Node(*it);
      assert(node->color == Node::RED);
      prev->children[1] = node;
      node->parent = prev;
      insert_fixup(node);
      prev = node;
    }
    size_ = last-first;
    reset_min();
  }

  size_t size() const {
    return size_;
  }

  Node *insert_front(Node *node) {
    ++size_;
    node->lnum = 0;
    node->mins[0] = node->mins[1] = MAX;
    if (!root) {
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
    return insert_front(new Node(x));
  }

  Node *insert_back(Node *node) {
    ++size_;
    node->lnum = 0;
    node->mins[0] = node->mins[1] = MAX;
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
    return insert_back(new Node(x));
  }

  Node *insert_next(Node *node, Node *dst) {
    if (!node) return root;

    node->children[0] = node->children[1] = nullptr;
    node->mins[0] = node->mins[1] = MAX;
    node->lnum = 0;

    if (!dst) {
      root = node;
      size_ = 1;
      node->color = Node::BLACK;
      node->parent = nullptr;
      return root;
    }

    node->color = Node::RED;
    if (dst->children[1]) {
      dst = dst->successor();
      assert(!dst->children[0]);
      dst->children[0] = node;
    } else {
      dst->children[1] = node;
    }
    node->parent = dst;

    reset_min(node);
    ++size_;
    propagate_lnum(node, 1);
    insert_fixup(node);
    return root;
  }

  Node *erase(Node *cur) {
    if (!cur) return nullptr;

    --size_;
    Node *y=cur;
    if (cur->children[0] && cur->children[1]) y = successor(y);
    assert(y);

    Node *x=y->children[0];
    if (!x) x = y->children[1];
    if (x) x->parent = y->parent;
    if (!y->parent) {
      root = x;
    } else {
      propagate_lnum(y, -1);
      int ydir=(y == y->parent->children[1]);
      y->parent->children[ydir] = x;
      y->parent->mins[ydir] = submin(x);
    }
    Node *xparent=y->parent;  // instead of x->parent; in case x is NIL
    bool fix_needed=(y->color == Node::BLACK);
    if (y != cur) {
      // keep detached node intact; DO NOT cur->value = y->value.
      assert(!y->children[0]);

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
        if (x) x->parent = y;
      }

      if (y != xparent) {
        // seems redundant, but not, I'm confused
        y->mins[0] = submin(y->children[0]);
        y->mins[1] = submin(y->children[1]);
        reset_min(y);
      }
    }
    if (xparent) {
      xparent->mins[0] = submin(xparent->children[0]);
      xparent->mins[1] = submin(xparent->children[1]);
      reset_min(xparent);
    }

    if (fix_needed) {
      erase_fixup(x, xparent);
    }

    cur->color = Node::RED;
    cur->children[0] = cur->children[1] = cur->parent = nullptr;
    cur->lnum = 0;
    cur->mins[0] = cur->mins[1] = MAX;
    return root;
  }

  Node *merge(RedBlackTree &oth) {
    if (!oth.root) return root;
    if (!root) {
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
    if (!med) {
      merge(oth);
      return root;
    }

    if (!root && !oth.root) {
      size_ = 1;
      root = med;
      root->color = Node::BLACK;
      root->lnum = 0;
      root->mins[0] = root->mins[1] = MAX;
      return root;
    }

    med->color = Node::RED;
    if (!root) {
      oth.insert_front(med);
      swap(oth);
      return root;
    }
    if (!oth.root) {
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

      if (!med->parent) {
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

    med->mins[0] = submin(med->children[0]);
    med->mins[1] = submin(med->children[1]);
    reset_min(med);

    med->lnum = calc_size(med->children[0]);
    size_ += oth.size_+1;
    oth.release();
    insert_fixup(med);
    return root;
  }

  std::pair<RedBlackTree, RedBlackTree> split(Node *crit) {
    if (!crit) {
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
      if (crit != root)
        crit_dir = (crit == crit->parent->children[1]);
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

  Node *node_at(size_t pos, const T &val) {
    Node *node=node_at(pos);
    node->value = val;
    reset_min(node);
    return node;
  }

  Node *cshift(size_t il, size_t ir) {
#if 1  /* insert/erase-based circular shift */
    /* XXX */
    Node *mr=node_at(ir);
    erase(mr);
    if (il == 0) {
      insert_front(mr);
    } else {
      insert_next(mr, node_at(il-1));
    }
    return root;

#else  /* merge/split-based circular shift */
    Node *lm=node_at(il), *mr=node_at(ir);

    RedBlackTree left, mid, right, tmp;
    std::tie(left, tmp) = split(lm);
    swap(tmp);
    std::tie(mid, right) = split(mr);

    mid.insert_front(lm);
    mid.merge(right);
    left.merge(mid, mr);
    swap(left);
    return root;

#endif
  }

  T min(size_t il, size_t ir) const {
    return min(root, il, ir);
  }

  /************************ Debug functions ************************/
  bool verify() const {
#ifdef RELEASE
    return true;
#endif

    auto report=[](const char *s) {
      fprintf(stderr, "%sviolated:%s %s%s%s\n",
              "\x1b[31;1m", "\x1b[0m", "\x1b[37;1m", s, "\x1b[0m");
    };

    bool violated=false;
    if (root->color == Node::RED) {
      violated = true;
      report("root node is red");
    }

    if (!root) {
      fprintf(stderr, "================================================\n");
      assert(size_ == 0);
      fprintf(stderr, "size: 0\n");
    }

    bool reached=false;
    size_t bheight=-1;
    size_t i=0;
    std::function<void (Node *, int, size_t)> dfs=[&](
        Node *subroot, int depth, size_t bh) {

      if (subroot->color == Node::RED) {
        // Note: even if violated, we continue checking.  We may not assume
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

      // XXX only suitable for [T = int]
      fprintf(stderr, "%zu\t%s%*s%d%s (%zu) [%zu] {%d, %d}%s\n",
              size_-1-i,
              is_red(subroot)? "\x1b[31;1m":"\x1b[37;1m",
              depth*2+2, "  ", subroot->value,
              "\x1b[0m", bh, subroot->lnum, 
              subroot->mins[0], subroot->mins[1],
              (left && right)? "":" *");

      ++i;
      if (left) dfs(left, depth+1, bh);
    };

    fprintf(stderr, "================================================\n");
    fprintf(stderr, "size: %zu\n", size_);
    dfs(root, 0, 0);
    fprintf(stderr, "================================================\n");
    return !violated;
  }
  /************************ End of debug functions ************************/
};

int main() {
  size_t n;
  int q;
  scanf("%zu %d", &n, &q);

  std::vector<int> a(n);
  for (size_t i=0; i<n; ++i)
    scanf("%d", &a[i]);

  RedBlackTree<int> rb(a.begin(), a.end());

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
      printf("%d\n", rb.min(l, r));
    } else if (t == 2) {
      size_t pos;
      int val;
      scanf("%zu %d", &pos, &val);
      rb.node_at(pos, val);
    }
  }
}
