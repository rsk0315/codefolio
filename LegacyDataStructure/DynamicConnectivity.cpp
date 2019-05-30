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
#define fprintf(...) void(0)

class EulerTourForest;

template <class T>
class RedBlackTree {
  friend EulerTourForest;

  size_t size_=0;
  struct Node {
    Node *children[2]={nullptr, nullptr}, *parent=nullptr;
    T value;
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

  std::pair<Node *, Node *> split(Node *crit) {
    if (crit == nullptr) {
      return {root, nullptr};
    }

    RedBlackTree left(crit->children[0]), right(crit->children[1]);
    crit->children[0] = crit->children[1] = nullptr;

    if (crit == root) {
      swap(left);
      return {root, right.root};
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

    left.reset_size();
    right.reset_size();

    swap(left);
    return {root, right.root};
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

      fprintf(stderr, "%s%*s(%zu, %zu)%s (%zu) [%zu]%s\n",
              is_red(subroot)? "\x1b[31;1m":"\x1b[37;1m",
              depth*2+2, "  ", subroot->value.first, subroot->value.second,
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

class EulerTourForest {
  using Edge = std::pair<size_t, size_t>;
  using RbTree = RedBlackTree<Edge>;
  using RbNode = typename RbTree::Node;
  std::map<Edge, RbNode *> nodes;
  std::map<RbNode *, RbTree *> trees;

  RbNode *insert_if_needed(size_t u) {
    Edge uu(u, u);
    auto it=nodes.lower_bound(uu);
    if (it != nodes.end() && it->first == uu)
      return it->second->root();

    RbNode *root=new RbNode(uu);
    nodes.emplace_hint(it, uu, root);
    trees.emplace(root, new RbTree(root));
    return root;
  }

public:
  EulerTourForest() = default;

  EulerTourForest(size_t n) {
    for (size_t i=0; i<n; ++i) {
      Edge e(i, i);
      RbNode *root=new RbNode(e);
      nodes.emplace(e, root);
      trees.emplace(root, new RbTree(root));
    }
  }

  bool connected(size_t u, size_t v) const {
    auto it_u=nodes.find({u, u});
    if (it_u == nodes.end()) return false;
    auto it_v=nodes.find({v, v});
    if (it_v == nodes.end()) return false;
    return it_u->second->root() == it_v->second->root();
  }

  size_t size() const {
    return trees.size();
  }

  RbNode *reroot(size_t u) {
    // {X, *}, ..., {*, u}, {u, u}, {u, *}, ..., {*, X}
    // -> {u, u}, {u, *}, ... {*, X}, {X, *}, ..., {*, u}
    Edge e(u, u);
    RbNode *edge=nodes.at(e);
    RbNode *old_root=edge->root();
    RbTree *old_tree=trees.at(old_root);
    if (old_tree->first()->value.first == u) return old_root;

    trees.erase(old_root);
    std::pair<RbNode *, RbNode *> tmp=old_tree->split(edge);
    delete old_tree;

    RbTree former(tmp.first), latter(tmp.second);
    latter.merge(former);
    RbNode *new_root=latter.insert_back(edge);
    RbTree *new_tree=new RbTree(new_root);
    trees.emplace(new_root, new_tree);
    return new_root;
  }

  bool link(size_t u, size_t v) {
    Edge uu(u, u), vv(v, v);
    RbNode *u_root=insert_if_needed(u);
    RbNode *v_root=insert_if_needed(v);
    if (u_root == v_root) return false;

    u_root = reroot(u);
    v_root = reroot(v);
    RbTree *u_tree=trees.at(u_root), *v_tree=trees.at(v_root);
    // FIXME redundant mutations
    trees.erase(u_root);
    trees.erase(v_root);

    Edge uv(u, v), vu(v, u);
    RbNode *uv_edge=new RbNode(uv), *vu_edge=new RbNode(vu);
    nodes.emplace(uv, uv_edge);
    nodes.emplace(vu, vu_edge);

    u_tree->merge(*v_tree, uv_edge);
    RbNode *new_root=u_tree->insert_back(vu_edge);
    delete u_tree;
    delete v_tree;
    
    RbTree *new_tree=new RbTree(new_root);
    trees.emplace(new_root, new_tree);

    return true;
  }

  bool cut(size_t u, size_t v) {
    if (nodes.find({u, u}) == nodes.end() || nodes.find({v, v}) == nodes.end())
      return false;

    reroot(u);
    Edge uv(u, v), vu(v, u);
    auto it=nodes.find(uv);
    if (it == nodes.end()) return false;

    RbNode *uv_edge=it->second, *vu_edge=nodes.at(vu);
    RbNode *old_root=uv_edge->root();
    RbTree *old_tree=trees.at(old_root);
    nodes.erase(uv);
    nodes.erase(vu);
    trees.erase(old_root);

    std::pair<RbNode *, RbNode *> tmp=old_tree->split(uv_edge);
    delete old_tree;
    RbNode *left=tmp.first;
    tmp = RbTree(tmp.second).split(vu_edge);

    RbTree *u_tree=new RbTree(left), u_tree_right(tmp.second);
    u_tree->merge(u_tree_right);
    RbTree *v_tree=new RbTree(tmp.first);

    trees.emplace(u_tree->root, u_tree);
    trees.emplace(v_tree->root, v_tree);

    nodes.emplace(Edge(u, u), u_tree->root);
    nodes.emplace(Edge(v, v), v_tree->root);
    return true;
  }

  std::vector<Edge> edges(size_t u) const {
    auto it=nodes.find({u, u});
    if (it == nodes.end()) return {};

    std::vector<Edge> res;
    const RbNode *u_first=it->second->first();
    for (const RbNode *p=u_first; p; p=p->successor())
      if (p->value.first < p->value.second)
        res.push_back(p->value);

    return res;
  }

  std::vector<Edge> edges(size_t u, size_t v) const {
    auto u_it=nodes.find({u, u});
    if (u_it == nodes.end()) return {};
    auto v_it=nodes.find({v, v});
    if (v_it == nodes.end()) return {};

    RbNode *u_root=u_it->second->root();
    RbNode *v_root=v_it->second->root();
    RbTree *u_tree=trees.at(u_root);
    RbTree *v_tree=trees.at(v_root);

    if (u_tree->size() > v_tree->size()) return edges(v);
    return edges(u);
  }

  void debug() const {
    for (const auto &p: trees) {
      const RbTree *tree=p.second;
      const RbNode *node=tree->first();
      while (node != nullptr) {
        fprintf(stderr, " (%zu, %zu)", node->value.first, node->value.second);
        node = node->successor();
      }
      fprintf(stderr, "\n");
      tree->verify();
    }
  }
};

class DynamicConnectivity {
  using Edge = std::pair<size_t, size_t>;
  size_t n, k;
  std::vector<EulerTourForest> fs;
  std::vector<std::set<Edge>> aux;

public:
  DynamicConnectivity(size_t n):
    n(n), k(1), fs(1, EulerTourForest(n)), aux(1)
  {}

  bool connected(size_t u, size_t v) const {
    return fs[0].connected(u, v);
  }

  size_t size() const {
    return fs[0].size();
  }

  void link(size_t u, size_t v) {
    fprintf(stderr, "link(%zu, %zu)\n", u, v);
    if (connected(u, v)) {
      aux[0].emplace(std::minmax(u, v));
      return;
    }
    fs[0].link(u, v);
  }

  void cut(size_t u, size_t v) {
    fprintf(stderr, "cut(%zu, %zu)\n", u, v);
    for (size_t i=0; i<k; ++i) {
      auto it=aux[i].find(std::minmax(u, v));
      if (it != aux[i].end()) {
        // untouch the spanning forest
        aux[i].erase(it);
        return;
      }
    }

    assert(fs[0].cut(u, v));
    size_t i;
    for (i=1; i<k; ++i)
      if (!fs[i].cut(u, v)) break;

    std::vector<Edge> up=fs[i-1].edges(u, v);
    if (i == k) {
      ++k;
      fs.emplace_back();
      aux.emplace_back();
    }
    for (const auto &p: up)
      fs[i].link(p.first, p.second);

    for (size_t j=k; j--;) {
      for (auto it=aux[j].begin(); it!=aux[j].end();) {
        size_t uu=it->first, vv=it->second;

        if ((connected(u, uu) || connected(u, vv))
            && (connected(v, uu) || connected(v, vv))) {

          aux[j].erase(it);
          fs[j].link(uu, vv);
          while (j--) fs[j].link(uu, vv);
          return;
        }

        if (j+1 <= k-1) {
          // XXX should be ++it if {uu, vv} are not incident to T_u or T_v
          aux[j+1].insert(*it);
          it = aux[j].erase(it);
        } else {
          ++it;
        }
      }
    }
  }

  void debug() const {
    for (size_t i=0; i<fs.size(); ++i) {
      fprintf(stderr, "F%zu:\n", i);
      fs[i].debug();
    }
    for (size_t i=0; i<k; ++i) {
      fprintf(stderr, "aux (of spec %zu):\n", i);
      for (auto it=aux[i].cbegin(); it!=aux[i].cend(); ++it) {
        fprintf(stderr, " {%zu, %zu}", it->first, it->second);
      }
      fprintf(stderr, "\n");
    }
  }
};

int solve() {
#ifdef ONLINE_JUDGE
  freopen("connect.in", "r", stdin);
  freopen("connect.out", "w", stdout);
#endif

  size_t N;
  int Q;
  scanf("%zu %d", &N, &Q);

  if (Q == 0) return 0;

  DynamicConnectivity dc(N);
  for (int i=0; i<Q; ++i) {
    fprintf(stderr, "Query #%d\n", i);

    char op;
    scanf(" %c", &op);
    if (op == '+') {
      size_t u, v;
      scanf("%zu %zu", &u, &v);
      --u;
      --v;
      dc.link(u, v);
    } else if (op == '-') {
      size_t u, v;
      scanf("%zu %zu", &u, &v);
      --u;
      --v;
      dc.cut(u, v);
    } else if (op == '?') {
      printf("%zu\n", dc.size());
    }

    // fprintf(stderr, "################################################\n");
    // dc.debug();
    // fprintf(stderr, "################################################\n\n");
  }
  return 0;
}

int main() {
  try {
    solve();
  // } catch (std::bad_alloc &e) {
  //   for (;;);
  } catch (std::out_of_range &e) {
    for (;;);
  }
}
