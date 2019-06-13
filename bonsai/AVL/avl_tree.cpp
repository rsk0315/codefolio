#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>
#include <memory>
#include <array>
#include <iterator>

template <typename Tp>
class avl_tree {
public:
  using value_type = Tp;
  using reference = value_type&;
  using const_reference = value_type const&;
  using difference_type = ptrdiff_t;
  using size_type = size_t;
  class iterator;
  class const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  struct M_node {
    value_type value{};
    size_type height = 0;
    using link_type = std::shared_ptr<M_node>;
    link_type parent = nullptr;
    std::array<link_type, 2> children{{nullptr, nullptr}};
    size_type left_size = 0;

    M_node() = default;
    M_node(value_type const& value): value(value) {}
  };

  using link_type = typename M_node::link_type;
  using base_ptr = std::shared_ptr<M_node>;
  using const_base_ptr = std::shared_ptr<M_node const>;

  size_type M_size = 0;
  base_ptr M_root = nullptr;
  base_ptr M_begin = nullptr;
  base_ptr M_end = nullptr;

public:
  class iterator {
    friend avl_tree;
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename avl_tree::value_type;
    using difference_type = typename avl_tree::difference_type;
    using pointer = std::shared_ptr<typename avl_tree::value_type>;
    using reference = typename avl_tree::reference;

  private:
    using base_ptr = typename avl_tree::base_ptr;
    base_ptr node = nullptr;

    iterator(const_base_ptr const& ptr): node(std::const_pointer_cast<M_node>(ptr)) {}

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(base_ptr const& ptr): node(ptr) {}
    iterator& operator =(iterator const&) = default;
    iterator& operator =(iterator&&) = default;

    iterator& operator +=(difference_type count) { S_advance(node, count); return *this; }
    iterator& operator -=(difference_type count) { S_advance(node, -count); return *this; }
    iterator& operator ++() { S_increment(node); return *this; }
    iterator& operator --() { S_decrement(node); return *this; }
    iterator operator +(difference_type count) const { return iterator(*this) += count; }
    iterator operator -(difference_type count) const { return iterator(*this) -= count; }
    iterator operator ++(int) { iterator tmp = *this; ++*this; return tmp; }
    iterator operator --(int) { iterator tmp = *this; --*this; return tmp; }
    difference_type operator -(const_iterator const& other) const { return S_index(*this) - S_index(other); }

    reference operator *() { return node->value; }

    bool operator <(const_iterator const& other) const { return S_index(*this) < S_index(other); }
    bool operator ==(const_iterator const& other) const { return node == other.node; }
    bool operator <=(const_iterator const& other) const { return !(other < *this); }
    bool operator >(const_iterator const& other) const { return (other < *this); }
    bool operator >=(const_iterator const& other) const { return !(*this < other); }
    bool operator !=(const_iterator const& other) const { return !(*this == other); }
  };

  class const_iterator {
    friend avl_tree;
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename avl_tree::value_type;
    using difference_type = typename avl_tree::difference_type;
    using pointer = std::shared_ptr<typename avl_tree::value_type const>;
    using reference = typename avl_tree::const_reference;

  private:
    using base_ptr = typename avl_tree::const_base_ptr;
    using mutable_base_ptr = typename avl_tree::base_ptr;
    base_ptr node = nullptr;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    const_iterator(iterator const& other): node(std::const_pointer_cast<M_node const>(other.node)) {}
    const_iterator(iterator&& other): node(std::const_pointer_cast<M_node const>(other.node)) {}
    const_iterator(base_ptr const& ptr): node(ptr) {}
    const_iterator(mutable_base_ptr const& ptr): node(std::const_pointer_cast<M_node const>(ptr)) {}
    
    const_iterator& operator =(const_iterator const&) = default;
    const_iterator& operator =(const_iterator&&) = default;

    const_iterator& operator +=(difference_type count) { S_advance(node, count); return *this; }
    const_iterator& operator -=(difference_type count) { S_advance(node, -count); return *this; }
    const_iterator& operator ++() { S_increment(node); return *this; }
    const_iterator& operator --() { S_decrement(node); return *this; }
    const_iterator operator +(difference_type count) const { return const_iterator(*this) += count; }
    const_iterator operator -(difference_type count) const { return const_iterator(*this) -= count; }
    const_iterator operator ++(int) { const_iterator tmp = *this; ++*this; return tmp; }
    const_iterator operator --(int) { const_iterator tmp = *this; --*this; return tmp; }
    difference_type operator -(const_iterator const& other) const { return S_index(*this) - S_index(other); }

    reference operator *() const { return node->value; }

    bool operator <(const_iterator const& other) const { return S_index(*this) < S_index(other); }
    bool operator ==(const_iterator const& other) const { return node == other.node; }
    bool operator <=(const_iterator const& other) const { return !(other < *this); }
    bool operator >(const_iterator const& other) const { return (other < *this); }
    bool operator >=(const_iterator const& other) const { return !(*this < other); }
    bool operator !=(const_iterator const& other) const { return !(*this == other); }
  };


private:
  static difference_type S_balance_factor(const_base_ptr node) {
    return S_balance_factor(std::const_pointer_cast<M_node>(node));
  }
  static difference_type S_balance_factor(base_ptr node) {
    std::array<difference_type, 2> height = {};
    for (size_t i = 0; i <= 1; ++i)
      if (node->children[i])
        height[i] = node->children[i]->height;

    return height[1] - height[0];
  }

  static void S_fix_height_one(base_ptr pos) {
    pos->height = 0;
    for (auto child: pos->children)
      if (child) pos->height = std::max(pos->height, child->height+1);
  }

  static void S_fix_height(base_ptr pos) {
    while (pos) {
      S_fix_height_one(pos);
      pos = pos->parent;
    }
  }

  static void S_fix_left_subtree_size(base_ptr pos, difference_type diff) {
    while (pos->parent) {
      if (pos == pos->parent->children[0])
        pos->parent->left_size += diff;
      pos = pos->parent;
    }
  }

  void M_prepare_sentinel() {
    assert(!M_root && !M_begin && !M_end);
    M_root = M_begin = M_end = base_ptr(new M_node);
  }

  static void S_clear_dfs(base_ptr root) {
    for (auto& child: root->children) {
      if (child) {
        S_clear_dfs(child);
        child = nullptr;
      }
    }
  }

  void M_clear() {
    M_size = 0;
    S_clear_dfs(M_root);
    M_begin = M_end = M_root = nullptr;
    M_prepare_sentinel();
  }

  static base_ptr S_leftmost(base_ptr pos) {
    while (pos->children[0]) pos = pos->children[0];
    return pos;
  }
  static base_ptr S_rightmost(base_ptr pos) {
    while (pos->children[1]) pos = pos->children[1];
    return pos;
  }

  static void S_increment(const_base_ptr& pos) { S_neighbor(pos, 1); }
  static void S_increment(base_ptr& pos) {
    const_base_ptr tmp = std::const_pointer_cast<M_node const>(pos);
    S_increment(tmp);
    pos = std::const_pointer_cast<M_node>(tmp);
  }
  static void S_decrement(const_base_ptr& pos) { S_neighbor(pos, 0); }
  static void S_decrement(base_ptr& pos) {
    const_base_ptr tmp = std::const_pointer_cast<M_node const>(pos);
    S_decrement(tmp);
    pos = std::const_pointer_cast<M_node>(tmp);
  }
  static void S_neighbor(const_base_ptr& pos, size_t dir) {
    if (pos->children[dir]) {
      pos = pos->children[dir];
      while (pos->children[!dir]) pos = pos->children[!dir];
      return;
    }
    while (true) {
      if (pos == pos->parent->children[!dir]) {
        pos = pos->parent;
        return;
      }
      pos = pos->parent;
    }
  }

  static void S_advance(const_base_ptr& pos, difference_type count = +1) {
    if (count > 0) {
      while (pos->parent) {
        difference_type tmp = count + S_index_diff_to_parent(pos);
        if (tmp < 0) break;
        count = tmp;
        pos = pos->parent;
      }
    } else /* if (count < 0) */ {
      while (pos->parent) {
        difference_type tmp = count + S_index_diff_to_parent(pos);
        if (tmp > 0) break;
        count = tmp;
        pos = pos->parent;
      }
    }

    while (count != 0) {
      if (count > 0) {
        pos = pos->children[1];
        count -= pos->left_size + 1;
      } else /* if (count < 0) */ {
        pos = pos->children[0];
        count += pos->parent->left_size - pos->left_size;
      }
    }
  }
  static void S_advance(base_ptr& pos, difference_type count = +1) {
    const_base_ptr tmp = std::const_pointer_cast<M_node const>(pos);
    S_advance(tmp, count);
    pos = std::const_pointer_cast<M_node>(tmp);
  }

  static size_type S_index(const_iterator pos) { return S_index(pos.node); }
  static size_type S_index(base_ptr pos) { return S_index(std::const_pointer_cast<M_node const>(pos)); }
  static size_type S_index(const_base_ptr pos) {
    size_type res = pos->left_size;
    while (pos->parent) {
      if (pos == pos->parent->children[1])
        res += pos->parent->left_size + 1;
      pos = pos->parent;
    }
    return res;
  }

  static difference_type S_index_diff_to_parent(base_ptr const& pos) {
    return S_index_diff_to_parent(std::const_pointer_cast<M_node const>(pos));
  }
  static difference_type S_index_diff_to_parent(const_base_ptr const& pos) {
    if (pos == pos->parent->children[0]) {
      return -static_cast<difference_type>(pos->parent->left_size - pos->left_size);
    } else {
      return pos->left_size + 1;
    }
  }

  static void S_rotate(base_ptr pos, size_t dir, base_ptr& root) {
    base_ptr child = pos->children[!dir];
    pos->children[!dir] = child->children[dir];
    if (child->children[dir])
      child->children[dir]->parent = pos;

    child->parent = pos->parent;
    if (!pos->parent) {
      root = child;
    } else if (pos == pos->parent->children[dir]) {
      pos->parent->children[dir] = child;
    } else {
      pos->parent->children[!dir] = child;
    }
    child->children[dir] = pos;
    pos->parent = child;

    if (dir == 0) {
      pos->parent->left_size += pos->left_size + 1;
    } else {
      pos->left_size -= pos->parent->left_size + 1;
    }
    // S_rotate() is intended to be called only by S_rebalance(),
    // which fix height more effectively
    // S_fix_height(pos);
  }
  void M_rotate(base_ptr pos, size_t dir) { S_rotate(pos, dir, M_root); }

  void M_rebalance(base_ptr pos) { S_rebalance(pos, M_root); }
  static void S_rebalance(base_ptr pos, base_ptr& root) {
    base_ptr start = pos;
    do {
      while (pos) {
        S_fix_height_one(pos);
        int bf = S_balance_factor(pos);
        if (!(-1 <= bf && bf <= +1)) break;
        pos = pos->parent;
      }
      if (!pos) return;

      size_t cdir = (S_balance_factor(pos) > 0);
      base_ptr child = pos->children[cdir];
      int bf = S_balance_factor(child);
      size_t gdir = ((cdir == 0)? (bf >= 0) : (bf > 0));
      base_ptr gchild = child->children[gdir];

      if (cdir == gdir) {
        S_rotate(pos, !cdir, root);
        S_fix_height_one(pos);
        pos = child;
      } else {
        S_rotate(child, cdir, root);
        S_rotate(pos, gdir, root);
        S_fix_height_one(child);
        S_fix_height_one(pos);
        pos = gchild;
      }
    } while (true);
  }

  void M_node_swap(base_ptr lhs, base_ptr rhs) {
    // swaps the position of two nodes, but corresponding iterators remain valid
    S_node_swap(lhs, rhs, M_begin, M_end, M_root);
  }
  static void S_node_swap(base_ptr lhs, base_ptr rhs, base_ptr& begin, base_ptr& end, base_ptr& root) {
    if (root == lhs) {
      root = rhs;
    } else if (root == rhs) {
      root = lhs;
    }
    if (begin == lhs) {
      begin = rhs;
    } else if (begin == rhs) {
      begin = lhs;
    }
    if (end == lhs) {
      end = rhs;
    } else if (end == rhs) {
      end = lhs;
    }

    for (auto child: lhs->children)
      if (child) child->parent = rhs;
    for (auto child: rhs->children)
      if (child) child->parent = lhs;

    base_ptr lpar = lhs->parent;
    if (lpar) {
      size_type ldir = (lhs == lpar->children[1]);
      lpar->children[ldir] = rhs;
    }
    base_ptr rpar = rhs->parent;
    if (rpar) {
      size_type rdir = (rhs == rpar->children[1]);
      rpar->children[rdir] = lhs;
    }

    std::swap(lhs->children, rhs->children);
    std::swap(lhs->parent, rhs->parent);
    std::swap(lhs->left_size, rhs->left_size);
    std::swap(lhs->height, rhs->height);
  }

  base_ptr M_insert(const_base_ptr pos, base_ptr newnode) {
    return M_insert(std::const_pointer_cast<M_node>(pos), newnode);
  }
  base_ptr M_insert(base_ptr pos, base_ptr newnode) {
    ++M_size;
    return S_insert(pos, newnode, M_begin, M_root);
  }
  base_ptr S_insert(base_ptr pos, base_ptr newnode, base_ptr& begin, base_ptr& root) {
    if (pos->children[0]) {
      S_decrement(pos);
      pos->children[1] = newnode;
    } else {
      pos->children[0] = newnode;
      if (pos == begin) begin = newnode;
    }
    newnode->parent = pos;
    S_fix_height(pos);
    S_fix_left_subtree_size(newnode, +1);
    S_rebalance(newnode, root);
    return newnode;
  }

  base_ptr M_erase(const_base_ptr pos) { return M_erase(std::const_pointer_cast<M_node>(pos)); }
  base_ptr M_erase(base_ptr pos) {
    base_ptr next = S_erase(pos, M_begin, M_end, M_root);
    --M_size;
    return next;
  }
  static base_ptr S_erase(base_ptr pos, base_ptr& begin, base_ptr& end, base_ptr& root) {
    if (pos->children[0] && pos->children[1]) {
      base_ptr tmp = pos;
      S_advance(tmp);
      // pos->value = std::move(tmp->value);
      // pos = tmp;
      S_node_swap(pos, tmp, begin, end, root);
      // S_inspect_dfs(root);
    }

    base_ptr next = pos;
    if (next != end) S_increment(next);
    if (pos == begin) begin = next;

    base_ptr child = pos->children[0];
    if (!child) child = pos->children[1];

    if (child) {
      child->parent = pos->parent;
    } else if (pos == end) {
      // next = end = pos->parent;
      assert(false);
      next = /* end = */ pos->parent;
    }

    if (!pos->parent) {
      root = child;
    } else {
      S_fix_left_subtree_size(pos, -1);
      size_type dir = (pos == pos->parent->children[1]);
      pos->parent->children[dir] = child;
      S_fix_height(pos->parent);
      S_rebalance(pos->parent, root);
    }
    return next;
  }

  static void S_check(base_ptr pos) {
    if (!pos) {
      fprintf(stderr, "(nil)\n");
    } else {
      size_t h[2] = {};
      for (size_t i = 0; i <= 1; ++i)
        if (pos->children[i]) h[i] = pos->children[i]->height;
      fprintf(stderr, "node at %p, height[%zu, %zu], value{%d}\n",
              pos.get(), h[0], h[1], pos->value);
    }
  }

  static void S_inspect_dfs(const const_base_ptr& root, size_type depth = 0) {
    auto child = root->children[1];
    if (child) S_inspect_dfs(child, depth+1);
    fprintf(stderr, "%*s-(%p) (%zu): %d (H: %zu) (P: %p)\n",
            static_cast<int>(depth), "",
            root.get(),
            root->left_size,
            root->value,
            root->height,
            root->parent.get()
            );
    child = root->children[0];
    if (child) S_inspect_dfs(child, depth+1);
  }

  base_ptr M_merge(avl_tree&& other) {
    if (other.empty()) return M_end;
    if (empty()) {
      *this = std::move(other);
      return M_begin;
    }
    size_type left_size = M_size + 1;  // +1 for M_end
    M_size += other.M_size + 2;  // +1 for dummy, +1 for M_end
    base_ptr med(new M_node);
    M_root = S_merge(M_root, other.M_root, med, left_size);

    M_node_swap(M_end, other.M_end);
    base_ptr tmp(new M_node);
    M_node_swap(tmp, other.M_end);
    M_erase(tmp);
    med = M_erase(med);

    other.M_size = 0;
    other.M_root = other.M_begin = other.M_end;
    return med;
  }

  static base_ptr S_merge(base_ptr left, base_ptr right, base_ptr med, size_type left_size) {
    base_ptr left_root = left;
    base_ptr right_root = right;
    base_ptr root = med;
    if (left->height < right->height) {
      size_t h = left->height + 1;
      while (right->height > h) right = right->children[0];  // left spine
      med->parent = right->parent;
      right->parent->children[0] = med;
      root = right_root;
    } else if (left->height > right->height) {
      size_t h = right->height + 1;
      while (left->height > h) {
        left_size -= left->left_size + 1;
        left = left->children[1];  // right spine
      }
      med->parent = left->parent;
      left->parent->children[1] = med;
      root = left_root;
    }

    left->parent = right->parent = med;
    med->children[0] = left;
    med->children[1] = right;
    med->left_size = left_size;
    S_fix_left_subtree_size(med, +1);
    S_fix_height(med);
    S_rebalance(med, root);
    return root;
  }

  // avl_tree M_split(base_ptr pos) {
  //   avl_tree right;
  //   right->root = S_split(M_root, pos);
  //   // TODO M_size の管理
  //   right->M_begin = S_leftmost(right->root);
  //   right->M_end = S_rightmost(right->root);
  //   return right;
  // }
  // static base_ptr S_split(base_ptr root, const_base_ptr pos) {
  //   base_ptr left_root = pos->children[0];
  //   base_ptr right_root = pos->children[1];
  //   left->root->parent = right_root->parent = nullptr;
  //   pos->children[0] = pos->children[1] = nullptr;
  //   base_ptr next = pos->parent;
  //   // TODO 根まで登りながら左と右に merge
  //   //      size を管理しながらやる
  //   //      計算量が O(log n) になるように気をつける

  //   // この呼び出しのあと root から辿れる木は，元々の木で pos より真に左側に
  //   // あったもの．返り値は残りのノードに辿れる根．
  //   // あれれ，分離された木のサイズってどう取得するの？
  //   // → M_split 側で pos-M_begin から先に計算しておくとよい
  // }

  static void S_deep_copy_dfs(base_ptr& dst, base_ptr const& src) {
    dst = base_ptr(new M_node(src->value));
    dst->parent = src->parent;
    dst->left_size = src->left_size;
    dst->height = src->height;
    for (size_t i = 0; i <= 1; ++i) {
      if (src->children[i])
        S_deep_copy_dfs(dst->children[i], src->children[i]);
    }
  }

  void M_deep_copy(avl_tree const& other) {
    fprintf(stderr, "deep-copied\n");
    M_size = other.M_size;
    S_deep_copy_dfs(M_root, other.M_root);
    M_begin = S_leftmost(M_root);
    M_end = S_rightmost(M_root);
  }

public:
  avl_tree() { M_prepare_sentinel(); }
  avl_tree(avl_tree const& other) { M_deep_copy(other); }
  avl_tree(avl_tree&&) = default;
  avl_tree(size_type count, value_type const& value = value_type{}): avl_tree() {
    while (count--) push_back(value);
  }
  template <typename InputIt, typename = std::_RequireInputIter<InputIt>>
  avl_tree(InputIt first, InputIt last): avl_tree() {
    while (first != last) push_back(*first++);
  }
  avl_tree(std::initializer_list<value_type> ilist): avl_tree(ilist.begin(), ilist.end()) {}

  avl_tree& operator =(avl_tree const& other) { M_deep_copy(other); }
  avl_tree& operator =(avl_tree&&) = default;

  // # element access
  // ## at
  reference at(size_t i) {
    if (i >= size()) throw std::out_of_range("i >= size()");
    return *(begin() + i);
  }
  const_reference at(size_t i) const {
    if (i >= size()) throw std::out_of_range("i >= size()");
    return (*cbegin() + i);
  }
  // ## operator []
  reference operator [](size_t i) { return *(begin() + i); }
  const_reference operator [](size_t i) const { return *(cbegin() + i); }
  // ## front and back
  reference first() { return *begin(); }
  const_reference first() const { return *cbegin(); }
  reference back() { return *rbegin(); }
  const_reference back() const { return *crbegin(); }

  // # iterators
  // ## begin
  iterator begin() { return M_begin; }
  const_iterator begin() const { return M_begin; }
  const_iterator cbegin() const { return M_begin; }
  // ## end
  iterator end() { return M_end; }
  const_iterator end() const { return M_end; }
  const_iterator cend() const { return M_end; }
  // ## rbegin
  reverse_iterator rbegin() { return reverse_iterator(M_end); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(M_end); }
  const_reverse_iterator crbegin() const { return const_reverse_iterator(M_end); }
  // ## rend
  reverse_iterator rend() { return reverse_iterator(M_begin); }
  const_reverse_iterator rend() const { return const_reverse_iterator(M_begin); }
  const_reverse_iterator crend() const { return const_reverse_iterator(M_begin); }
  // ## lower_bound
  template <typename Comparator>
  iterator lower_bound(value_type const& value, Comparator comp) {
    base_ptr pos = M_root;
    base_ptr res = pos;
    while (pos) {
      size_t dir = (pos != M_end) && comp(pos->value, value);
      if (dir == 0) res = pos;
      pos = pos->children[dir];
    }
    return res;
  }
  iterator lower_bound(value_type const& value) { return lower_bound(value, std::less<value_type>()); }
  // ## upper_bound
  template <typename Comparator>
  iterator upper_bound(value_type const& value, Comparator comp) {
    base_ptr pos = M_root;
    base_ptr res = pos;
    while (pos) {
      size_t dir = (pos != M_end) && !comp(value, pos->value);
      if (dir == 0) res = pos;
      pos = pos->children[dir];
    }
    return res;
  }
  iterator upper_bound(value_type const& value) { return upper_bound(value, std::less<value_type>()); }

  // # capacity
  // ## empty
  bool empty() const { return M_size == 0; }
  // ## size
  size_type size() const { return M_size; }

  // # modifiers
  // ## clear
  void clear() { M_clear(); }
  // ## insert and emplace
  iterator insert(const_iterator pos, value_type const& value) {
    return M_insert(pos.node, base_ptr(new M_node(value)));
  }
  iterator insert(const_iterator pos, value_type&& value) {
    return M_insert(pos.node, base_ptr(new M_node(std::move(value))));
  }
  iterator insert(const_iterator pos, size_t count, value_type const& value) {
    iterator res = pos;
    while (count--) res = M_insert(pos.node, base_ptr(new M_node(value)));
    return res;
  }
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    iterator res = pos;
    while (first != last) res = M_insert(pos.node, base_ptr(new M_node(*first++)));
    return res;
  }
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
    return insert(pos, ilist.begin(), ilist.end());
  }
  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return M_insert(pos.node, base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  // ## erase
  iterator erase(iterator pos) { return M_erase(pos.node); }
  iterator erase(const_iterator pos) { return M_erase(pos.node); }
  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) first = erase(first);
    return first;
  }
  // ## push_back and push_front
  void push_back(value_type const& value) { insert(cend(), value); }
  void push_back(value_type&& value) { insert(cend(), std::move(value)); }
  void push_front(value_type const& value) { insert(cbegin(), value); }
  void push_front(value_type&& value) { insert(cbegin(), std::move(value)); }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    M_insert(M_end, const_base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  template <typename... Args>
  void emplace_front(Args&&... args) {
    M_insert(M_begin, const_base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  // ## pop_back and pop_front
  void pop_back() { erase(--end()); }
  void pop_front() { erase(begin()); }
  // ## merge and split
  iterator merge(avl_tree&& other) { return M_merge(std::move(other)); }
  avl_tree split(const_iterator pos) {
    base_ptr right = M_split(pos);
    // update M_size, M_root, M_begin, and M_end of this
    return avl_tree(right);
  }

  // # debug
  // ## inspect
  void inspect() const {
    fprintf(stderr, "root: %p\n", M_root.get());
    fprintf(stderr, "begin: %p\n", M_begin.get());
    fprintf(stderr, "end: %p\n", M_end.get());
    fprintf(stderr, "size: %zu\n", M_size);
    S_inspect_dfs(std::const_pointer_cast<M_node const>(M_root));
  }

  void verify() const {
    fprintf(stderr, "begin: %p\n", M_begin.get());
    fprintf(stderr, "end: %p\n", M_end.get());
    bool violated = false;
    for (auto it = begin(); it != end(); ++it) {
      const_base_ptr node = it.node;
      // fprintf(stderr, "%p\n", node.get());
      // fprintf(stderr, "%d\n", node->value);
      int bf = S_balance_factor(node);
      if (!(-1 <= bf && bf <= +1)) {
        fprintf(stderr, "balance factor of %p (%d): %d\n",
                node.get(), node->value, bf);
        // assert(-1 <= bf && bf <= +1);
        violated = true;
      }
    }
    if (violated) inspect();
  }
};

int ITP2_7_C() {
  size_t q;
  scanf("%zu", &q);

  avl_tree<int> bst;
  for (size_t i = 0; i < q; ++i) {
    int t, x;
    scanf("%d %d", &t, &x);

    typename avl_tree<int>::const_iterator it = bst.lower_bound(x);
    if (t == 0) {
      // insert(x)
      if (it == bst.end() || *it != x) bst.insert(it, x);
      printf("%zu\n", bst.size());
    } else if (t == 1) {
      // find(x)
      printf("%d\n", (it != bst.end() && *it == x));
    } else if (t == 2) {
      // delete(x)
      if (it != bst.end() && *it == x) bst.erase(it);
    } else if (t == 3) {
      int y;
      scanf("%d", &y);
      while (it != bst.end() && *it <= y) printf("%d\n", *it++);
    }

    // bst.inspect();
    bst.verify();
  }
  return 0;
}

#include <random>

int random_test() {
  size_t n = 32;
  int m = 1023;
  avl_tree<int> bst;
  std::mt19937 rsk(0315);
  std::uniform_int_distribution<int> nya(0, m);
  for (size_t i = 0; i < n; ++i)
    bst.push_front(nya(rsk));

  bst.inspect();
  bst.verify();

  std::sort(bst.begin(), bst.end());
  bst.inspect();

  std::sort(bst.rbegin(), bst.rend());
  bst.inspect();

  for (size_t i = 0; i < n; ++i)
    printf("%d\n", bst[i]);

  return 0;
}

int main() {
  // ITP2_7_C();
  avl_tree<int> r{1, 2, 3}, s{4, 5};
  auto t = s;
  r.inspect();
  s.inspect();
  r.merge(std::move(s));
  r.inspect();
  // s.clear();
  t.inspect();
  s.inspect();
}
