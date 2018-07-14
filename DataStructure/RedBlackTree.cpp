#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <functional>

#define fprintf(...) void(0)

template <class T>
class RedBlackTree {
  struct Node {
    Node *left=nullptr, *right=nullptr, *parent=nullptr;
    T value;
    enum Color { Red=false, Black=true } color=Red;
    Node(T x): value(x) {}
  } *root=nullptr;
  size_t size_=0;

  void left_rotate(Node *cur) {
    Node *right=cur->right;
    assert(right != nullptr);
    cur->right = right->left;
    if (right->left != nullptr)
      right->left->parent = cur;
    right->parent = cur->parent;
    if (cur->parent == nullptr) {
      root = right;
    } else if (cur == cur->parent->left) {
      cur->parent->left = right;
    } else {
      cur->parent->right = right;
    }
    right->left = cur;
    cur->parent = right;
  }

  void right_rotate(Node *cur) {
    Node *left=cur->left;
    assert(left != nullptr);
    cur->left = left->right;
    if (left->right != nullptr)
      left->right->parent = cur;
    left->parent = cur->parent;
    if (cur->parent == nullptr) {
      root = left;
    } else if (cur == cur->parent->right) {
      cur->parent->right = left;
    } else {
      cur->parent->left = left;
    }
    left->right = cur;
    cur->parent = left;
  }

  bool is_red(Node *node) const {
    // NIL is black
    return node != nullptr && node->color == Node::Red;
  }

  Node *successor(Node *cur) const {
    Node *prev=nullptr;
    if (cur->right == nullptr) {
      while (cur->parent != nullptr) {
        prev = cur;
        cur = cur->parent;
        if (cur->left == prev) break;
      }
    }
    cur = cur->right;
    prev = nullptr;
    while (cur != nullptr) {
      prev = cur;
      cur = cur->left;
    }
    return prev;
  }
  
  Node *predecessor(Node *cur) const {
    Node *prev=nullptr;
    if (cur->left == nullptr) {
      while (cur->parent != nullptr) {
        prev = cur;
        cur = cur->parent;
        if (cur->right == prev) break;
      }
    }
    cur = cur->left;
    prev = nullptr;
    while (cur != nullptr) {
      prev = cur;
      cur = cur->right;
    }
    return prev;
  }
  
  void insert_fixup(Node *cur) {
    while (is_red(cur->parent)) {
      Node *gparent=cur->parent->parent;
      assert(gparent != nullptr);

      Node *uncle=gparent->right;
      bool mirrored=false;
      if (cur->parent == uncle) {
        uncle = gparent->left;
        mirrored = true;
      }
      if (is_red(uncle)) {
        cur->parent->color = uncle->color = Node::Black;
        gparent->color = Node::Red;
        cur = gparent;
      } else {
        if ((cur == cur->parent->right) ^ mirrored) {
          cur = cur->parent;
          mirrored? right_rotate(cur):left_rotate(cur);
        }
        cur->parent->color = Node::Black;
        cur->parent->parent->color = Node::Red;
        mirrored? left_rotate(gparent):right_rotate(gparent);
      }
    }
    root->color = Node::Black;
  }

  void erase_fixup(Node *cur, Node *parent) {
    while (cur != root && !is_red(cur)) {
      Node *sibling=parent->right;
      bool mirrored=false;
      if (cur == sibling) {
        sibling = parent->left;
        mirrored = true;
      }

      if (is_red(sibling)) {
        sibling->color = Node::Black;
        parent->color = Node::Red;
        mirrored? right_rotate(parent):left_rotate(parent);
        sibling = (mirrored? parent->left:parent->right);
      }

      if (sibling && !is_red(sibling->left) && !is_red(sibling->right)) {
        sibling->color = Node::Red;
        cur = parent;
        parent = cur->parent;
      } else {
        if (!mirrored) {
          if (sibling && !is_red(sibling->right)) {
            if (sibling->left != nullptr)
              sibling->left->color = Node::Black;

            sibling->color = Node::Red;
            right_rotate(sibling);
            sibling = parent->right;
          }
        } else {
          if (sibling && !is_red(sibling->left)) {
            if (sibling->right != nullptr)
              sibling->right->color = Node::Black;

            sibling->color = Node::Red;
            left_rotate(sibling);
            sibling = parent->left;
          }
        }
        if (sibling) sibling->color = parent->color;
        parent->color = Node::Black;
        if (sibling)
          (mirrored? sibling->left:sibling->right)->color = Node::Black;

        mirrored? right_rotate(parent):left_rotate(parent);
        cur = root;
        parent = nullptr;
      }
    }
    if (cur) cur->color = Node::Black;
  }

public:
  RedBlackTree() = default;

  size_t size() const {
    return size_;
  }

  bool insert(T x) {
    if (root == nullptr) {
      root = new Node(x);
      root->color = Node::Black;
      ++size_;
      return true;
    }

    Node **child=&root, *parent=nullptr;
    while (*child != nullptr) {
      parent = *child;
      if ((*child)->value > x) {
        child = &(*child)->left;
      } else if ((*child)->value < x) {
        child = &(*child)->right;
      } else {
        return false;
      }
    }
    *child = new Node(x);
    ++size_;
    (*child)->parent = parent;
    insert_fixup(*child);
    return true;
  }

  bool erase(T x) {
    Node *cur=root;
    while (cur != nullptr) {
      if (cur->value > x) {
        cur = cur->left;
      } else if (cur->value < x) {
        cur = cur->right;
      } else {
        break;
      }
    }
    if (cur == nullptr) return false;

    --size_;
    return erase(cur);
  }

  bool erase(Node *cur) {
    Node *y=cur;
    if (cur->left != nullptr && cur->right != nullptr) y = successor(cur);
    
    assert(y != nullptr);
    Node *x=y->left;
    if (x == nullptr) x = y->right;

    if (x != nullptr) x->parent = y->parent;
    if (y->parent == nullptr) {
      root = x;
    } else if (y == y->parent->left) {
      y->parent->left = x;
    } else {
      y->parent->right = x;
    }

    if (y != cur) {
      cur->value = y->value;
    }
    if (y->color == Node::Black) {
      erase_fixup(x, y->parent);
    }
    return true;
  }

  bool contains(T x) const {
    Node *cur=root;
    while (cur != nullptr) {
      if (cur->value < x) {
        cur = cur->right;
      } else if (cur->value > x) {
        cur = cur->left;
      } else {
        return true;
      }
    }
    return false;
  }

  bool verify() const {
    if (root == nullptr) return true;

    bool violated=false;
    if (root->color == Node::Red) {
      violated = true;
      fprintf(stderr, "root node is red\n");
    }
    
    bool reached=false;
    size_t bheight=-1;
    std::function<void (Node *, int, size_t)> dfs=[&](
        Node *subroot, int depth, size_t bh) {

      if (subroot->color == Node::Red) {
        if (subroot->parent && subroot->parent->color == Node::Red) {
          violated = true;
          fprintf(stderr, "two red nodes are connected\n");
        }
      } else {
        ++bh;
      }

      if (subroot->left == nullptr || subroot->right == nullptr) {
        if (!reached) {
          bheight = bh;
          reached = true;
        } else if (bheight != bh) {
          violated = true;
          fprintf(stderr, "different number of black nodes in two root-leaf paths\n");
        }
      }

      if (subroot->right != nullptr)
        dfs(subroot->right, depth+1, bh);

      fprintf(
          stderr, "%s%*s%d%s (%zu) %c\n",
          (subroot->color == Node::Red)? "\x1b[31;1m":"\x1b[37;1m",
          depth*2+2, "  ", subroot->value, "\x1b[0m", bh,
          (subroot->left&&subroot->right)? ' ':'*');

      if (subroot->left != nullptr)
        dfs(subroot->left, depth+1, bh);
    };
    fprintf(stderr, "================================================\n");
    dfs(root, 0, 0);
    fprintf(stderr, "================================================\n");
    return !violated;
  }
};

int main() {
  RedBlackTree<int> rbt;

  int q;
  scanf("%d", &q);
  for (int i=0; i<q; ++i) {
    int t, x;
    scanf("%d", &t);
    if (t == 0) {
      scanf("%d", &x);
      rbt.insert(x);
      printf("%zu\n", rbt.size());
    } else if (t == 1) {
      scanf("%d", &x);
      printf("%d\n", rbt.contains(x));
    } else if (t == 2) {
      scanf("%d", &x);
      rbt.erase(x);
    }
  }
  assert(rbt.verify());
}
