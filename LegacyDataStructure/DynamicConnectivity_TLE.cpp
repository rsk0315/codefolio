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

  size_t size(size_t u) const {
    auto it=nodes.find({u, u});
    if (it == nodes.end()) return 0;
    RbNode *root=it->second->root();
    RbTree *tree=trees.at(root);
    return tree->size();
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

  // bool contains(size_t u, size_t v) const {
  //   return nodes.find({u, v}) != nodes.end();
  // }

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
    if (size(u) > size(v)) return edges(v);
    return edges(u);
  }

  std::vector<size_t> component(size_t u) const {
    auto it=nodes.find({u, u});
    if (it == nodes.end()) return {};

    std::vector<size_t> res;
    const RbNode *u_first=it->second->first();
    for (const RbNode *p=u_first; p; p=p->successor()) {
      res.push_back(p->value.first);
      if (p->value.first != p->value.second)
        res.push_back(p->value.second);
    }
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());

    return res;
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
  // (n)t_edges[i][u] = {(non-)tree level i edges incident to u ...}
  std::vector<std::map<size_t, std::set<Edge>>> t_edges, nt_edges;
  std::map<Edge, size_t> level;

public:
  DynamicConnectivity(size_t n):
    n(n), k(1), fs(1, EulerTourForest(n)), t_edges(1), nt_edges(1)
  {}

  bool connected(size_t u, size_t v) const {
    return fs[0].connected(u, v);
  }

  size_t size() const {
    return fs[0].size();
  }

  void link(size_t u, size_t v) {
    fprintf(stderr, "link(%zu, %zu)\n", u, v);
    // level.emplace(std::minmax(u, v), 0);
    // ^ why does this cause `level.at(std::minmax(u, v)) == 1'?
    level[std::minmax(u, v)] = 0;
    fprintf(stderr, "level[%zu, %zu]: %zu\n",
            u, v, level.at(std::minmax(u, v)));
    if (!connected(u, v)) {
      fs[0].link(u, v);
      t_edges[0][u].emplace(u, v);
      t_edges[0][v].emplace(v, u);
    } else {
      nt_edges[0][u].emplace(u, v);
      nt_edges[0][v].emplace(v, u);
    }
  }

  void cut(size_t u, size_t v) {
    fprintf(stderr, "cut(%zu, %zu)\n", u, v);
    size_t ll=level.at(std::minmax(u, v));
    level.erase(std::minmax(u, v));
    {
      auto it=nt_edges[ll][u].find({u, v});
      if (it != nt_edges[ll][u].end()) {
        fprintf(stderr, "non-tree edge {%zu, %zu} is deleted\n", u, v);
        nt_edges[ll][u].erase(it);
        nt_edges[ll][v].erase({v, u});
        return;
      }
    }

    t_edges[ll][u].erase({u, v});
    t_edges[ll][v].erase({v, u});
    for (size_t i=ll+1; i--;)
      fs[i].cut(u, v);

    for (size_t i=ll+1; i--;) {
      if (fs[i].size(u) > fs[i].size(v))
        std::swap(u, v);

      auto tu=fs[i].component(u);

      for (size_t uu: tu) {
        for (auto it=t_edges[i][uu].begin(); it!=t_edges[i][uu].end();) {
          size_t x=it->first, y=it->second;
          ++it;
          if (x < y)
            // Note: this invalidates the iterator (it)
            increase_level(x, y, true);
        }
      }
      std::set<Edge> increased;
      for (size_t uu: tu) {
        for (auto it=nt_edges[i][uu].begin(); it!=nt_edges[i][uu].end();) {
          size_t x=it->first, y=it->second;
          fprintf(stderr, "Check {%zu, %zu}\n", x, y);
          if (!connected(y, v)) {
            ++it;
            if (increased.find(std::minmax(x, y)) == increased.end()) {
              increase_level(x, y, false);
              increased.emplace(std::minmax(x, y));
            }
            continue;
          }
          fprintf(stderr, "%zu and %zu are connected\n", y, v);
          fprintf(stderr, "{%zu, %zu} is now tree-edge\n", x, y);
          for (size_t j=i+1; j--;) {
            fs[j].link(x, y);
          }
          nt_edges[i][x].erase({x, y});
          nt_edges[i][y].erase({y, x});
          t_edges[i][x].emplace(x, y);
          t_edges[i][y].emplace(y, x);
          return;
        }
      }
    }
  }

  void increase_level(size_t u, size_t v, bool tree_edge) {
    size_t ll=level.at(std::minmax(u, v))++;
    fprintf(stderr, "increase {%zu, %zu} to %zu\n", u, v, ll+1);
    if (fs.size() < ll+2) fs.resize(ll+2);
    if (t_edges.size() < ll+2) t_edges.resize(ll+2);
    if (nt_edges.size() < ll+2) nt_edges.resize(ll+2);

    if (tree_edge) {
      fs[ll+1].link(u, v);

      t_edges[ll][u].erase({u, v});
      t_edges[ll][v].erase({v, u});
      t_edges[ll+1][u].emplace(u, v);
      t_edges[ll+1][v].emplace(v, u);
      return;
    }
    nt_edges[ll][u].erase({u, v});
    nt_edges[ll][v].erase({v, u});
    nt_edges[ll+1][u].emplace(u, v);
    nt_edges[ll+1][v].emplace(v, u);
  }

  void debug() const {
    for (size_t i=0; i<fs.size(); ++i) {
      fprintf(stderr, "F%zu:\n", i);
      fs[i].debug();
    }

    fprintf(stderr, "tree-edges[%zu], non-tree-edges[%zu]\n",
            t_edges.size(), nt_edges.size());

    fprintf(stderr, "level[%zu]\n", level.size());
    for (const auto &e_ll: level) {
      fprintf(stderr, "{%zu, %zu}: %zu\n",
              e_ll.first.first, e_ll.first.second,
              e_ll.second);
    }

    fprintf(stderr, "tree edges:\n");
    for (size_t ll=0; ll<t_edges.size(); ++ll) {
      for (const auto &p: t_edges[ll]) {
        for (const auto &e: p.second) {
          fprintf(stderr, "  {%zu, %zu} (%zu|", e.first, e.second, ll);
          fprintf(stderr, "%zu)\n", level.at(std::minmax(e.first, e.second)));
          assert(ll == level.at(std::minmax(e.first, e.second)));
        }
      }
    }

    fprintf(stderr, "non-tree edges:\n");
    for (size_t ll=0; ll<nt_edges.size(); ++ll) {
      for (const auto &p: nt_edges[ll]) {
        for (const auto &e: p.second) {
          fprintf(stderr, "  {%zu, %zu} (%zu|", e.first, e.second, ll);
          fprintf(stderr, "%zu)\n", level.at(std::minmax(e.first, e.second)));
          assert(ll == level.at(std::minmax(e.first, e.second)));
        }
      }
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

  // cin.scan(N, Q);
  scanf("%zu %d", &N, &Q);

  if (Q == 0) return 0;

  DynamicConnectivity dc(N);
  for (int i=0; i<Q; ++i) {
    fprintf(stderr, "Query #%d\n", i);

    // char op, sp;
    // cin.scan(op, sp);
    char op;
    scanf(" %c", &op);
    if (op == '+') {
      size_t u, v;
      // cin.scan(u, v);
      scanf("%zu %zu", &u, &v);
      --u;
      --v;
      dc.link(u, v);
    } else if (op == '-') {
      size_t u, v;
      // cin.scan(u, v);
      scanf("%zu %zu", &u, &v);
      --u;
      --v;
      dc.cut(u, v);
    } else if (op == '?') {
      fprintf(stderr, "size()?\n");
      // cout.println(dc.size());
      printf("%zu\n", dc.size());
    }

    if (false) {
      fprintf(stderr, "################################################\n");
      dc.debug();
      fprintf(stderr, "################################################\n\n");
    }
  }
  return 0;
}

int main() {
  // try {
    solve();
  // } catch (std::bad_alloc &e) {
  //   fprintf(stderr, "%s\n", e.what());
  //   for (;;);
  // } catch (std::out_of_range &e) {
  //   fprintf(stderr, "%s\n", e.what());
  //   for (;;);
  // }
}
