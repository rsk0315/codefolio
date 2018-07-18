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

// for release
// #define fprintf(...) void(0)

class EulerTourForest;

template <class T>
class RedBlackTree {
  friend EulerTourForest;

  struct Node {
    Node *children[2]={nullptr, nullptr}, *parent=nullptr;
    T value;
    enum Color { RED, BLACK } color=RED;
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

public:
  RedBlackTree() = default;

  Node *insert_front(Node *node) {
    if (root == nullptr) {
      node->color = Node::BLACK;
      root = node;
      return root;
    }
    Node *cur=first();
    cur->children[0] = node;
    node->parent = cur;
    node->color = Node::RED;
    insert_fixup(node);
    return root;
  }

  Node *insert_back(Node *node) {
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

    Node *y=cur;
    if (cur->children[0] && cur->children[1]) y = successor(y);
    assert(y != nullptr);

    Node *x=y->children[0];
    if (x == nullptr) x = y->children[1];

    if (x) x->parent = y->parent;
    if (y->parent == nullptr) {
      root = x;
    } else {
      y->parent->children[y == y->parent->children[1]] = x;
    }
    Node *xparent=y->parent;  // not x->parent; in case x is null
    bool fix_needed=(y->color == Node::BLACK);
    if (y != cur) {
      // keep detached node intact; not use cur->value = y->value.
      y->parent = cur->parent;
      y->children[0] = cur->children[0];
      y->children[1] = cur->children[1];
      y->color = cur->color;

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
    }

    if (fix_needed)
      erase_fixup(x, xparent);

    cur->children[0] = cur->children[1] = cur->parent = nullptr;
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
      root = med;
      med->color = Node::BLACK;
      return root;
    }
    med->color = Node::RED;
    if (root == nullptr) {
      oth.insert_front(med);
      swap(oth);
      return root;
    }
    if (oth.root == nullptr) {
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
    }

    oth.root = nullptr;
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

      fprintf(stderr, "%s%*s(%zu, %zu)%s (%zu)%s\n",
              is_red(subroot)? "\x1b[31;1m":"\x1b[37;1m",
              depth*2+2, "  ", subroot->value.first, subroot->value.second,
              "\x1b[0m", bh, (left && right)? "":" *");

      if (left) dfs(left, depth+1, bh);
    };

    fprintf(stderr, "================================================\n");
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

public:
  EulerTourForest(size_t n) {
    for (size_t i=0; i<n; ++i) {
      Edge e(i, i);
      RedBlackTree<Edge> *tree=new RedBlackTree<Edge>;
      RbNode *node=new RbNode(e);
      nodes.emplace(e, node);
      RbNode *root=tree->insert_front(node);
      trees.emplace(root, tree);
    }
  }

  bool connected(size_t u, size_t v) const {
    return nodes.at({u, u})->root() == nodes.at({v, v})->root();
  }

  size_t size() const {
    return trees.size();
  }

  void reroot(size_t u) {
    // {X, *}, ..., {*, u}, {u, u}, {u, *}, ..., {*, X}
    // -> {u, u}, {u, *}, ... {*, X}, {X, *}, ..., {*, u}
    Edge e(u, u);
    RbNode *edge=nodes.at(e);
    RbNode *old_root=edge->root();
    RbTree *old_tree=trees.at(old_root);
    if (old_tree->first()->value.first == u) return;

    trees.erase(old_root);
    std::pair<RbNode *, RbNode *> tmp=old_tree->split(edge);
    delete old_tree;

    RbTree former(tmp.first), latter(tmp.second);
    latter.merge(former);
    RbNode *new_root=latter.insert_back(edge);
    RbTree *new_tree=new RbTree(new_root);
    trees.emplace(new_root, new_tree);
  }

  bool link(size_t u, size_t v) {
    if (connected(u, v)) return false;

    reroot(u);
    reroot(v);
    Edge uu(u, u), vv(v, v);
    RbNode *u_root=nodes.at(uu)->root();
    RbNode *v_root=nodes.at(vv)->root();
    RbTree *u_tree=trees.at(u_root), *v_tree=trees.at(v_root);
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
    if (!connected(u, v)) return false;

    reroot(u);
    Edge uv(u, v), vu(v, u);
    RbNode *uv_edge=nodes.at(uv), *vu_edge=nodes.at(vu);
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
    std::vector<Edge> res;
    const RbNode *u_first=nodes.at(Edge(u, u))->first();
    for (const RbNode *p=u_first; p; p=p->successor())
      if (p->value.first < p->value.second)
        res.push_back(p->value);

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
    n(n), k(1), fs(k, EulerTourForest(n)), aux(k)
  {}

  bool connected(size_t u, size_t v) const {
    return fs[0].connected(u, v);
  }

  size_t size() const {
    return fs[0].size();
  }

  /*
    XXX: failed when cut(u, v) after link(u, v) if u and v are already
         connected at the time of the linking
    MWE:
        3 4
        + 1 2
        + 2 3
        + 3 1
        - 3 1
  */

  void link(size_t u, size_t v) {
    if (connected(u, v)) {
      aux[0].emplace(u, v);
      return;
    }
    fs[0].link(u, v);
  }

  void cut(size_t u, size_t v) {
    assert(fs[0].cut(u, v));
    size_t i;
    for (i=1; i<k; ++i)
      if (!fs[i].cut(u, v)) break;

    std::vector<Edge> up=fs[i-1].edges(u);
    if (i == k) {
      ++k;
      fs.emplace_back(n);
      aux.emplace_back();
    }
    for (const auto &p: up)
      fs[i].link(p.first, p.second);

    for (size_t j=i; j--;) {
      for (auto it=aux[j].begin(); it!=aux[j].end();) {
        size_t u=it->first, v=it->second;
        it = aux[j].erase(it);
        if (fs[j].connected(u, v)) {
          aux[j+1].emplace(u, v);
          continue;
        }

        fs[j].link(u, v);
        while (j--) fs[j].link(u, v);
        return;
      }
    }
  }
};

int main() {
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
  }
}
