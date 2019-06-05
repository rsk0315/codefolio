#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
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
    link_type parent = parent;
    std::array<link_type, 2> children = {nullptr, nullptr};
    size_type left_size = 0;
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
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename avl_tree::value_type;
    using difference_type = typename avl_tree::difference_type;
    using pointer = std::shared_ptr<typename avl_tree::value_type>;
    using reference = typename avl_tree::reference;

  private:
    using base_ptr = typename avl_tree::base_ptr;
    base_ptr node = nullptr;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(const_iterator const& other): node(other.node) {}
    iterator(const_iterator&& other): node(other.node) {}
    iterator& operator =(iterator const&) = default;
    iterator& operator =(iterator&&) = default;

    iterator& operator +=(difference_type count) { S_advance(node); return *this; }
    iterator& operator -=(difference_type count) { S_advance(-node); return *this; }
    iterator& operator ++() { S_increment(node); return *this; }
    iterator& operator --() { S_decrement(node); return *this; }
    iterator operator +(difference_type count) const { return iterator(*this) += count; }
    iterator operator -(difference_type count) const { return iterator(*this) -= count; }
    iterator operator ++(int) { iterator tmp = *this; ++tmp; return *this; }
    iterator operator --(int) { iterator tmp = *this; --tmp; return *this; }

    reference operator *() { return node->value; }

    bool operator <(const_iterator const& other) const { return S_index(*this) < S_index(other); }
    bool operator ==(const_iterator const& other) const { return node == other.node; }
    bool operator <=(const_iterator const& other) const { return !(other < *this); }
    bool operator >(const_iterator const& other) const { return (other < *this); }
    bool operator >=(const_iterator const& other) const { return !(*this < other); }
    bool operator !=(const_iterator const& other) const { return !(*this == other); }
  };

  class const_iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename avl_tree::value_type;
    using difference_type = typename avl_tree::difference_type;
    using pointer = std::shared_ptr<typename avl_tree::value_type const>;
    using reference = typename avl_tree::const_reference;

  private:
    using base_ptr = typename avl_tree::const_base_ptr;
    base_ptr node = nullptr;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    const_iterator& operator =(const_iterator const&) = default;
    const_iterator& operator =(const_iterator&&) = default;

    const_iterator& operator +=(difference_type count) { S_advance(node); return *this; }
    const_iterator& operator -=(difference_type count) { S_advance(-node); return *this; }
    const_iterator& operator ++() { S_increment(node); return *this; }
    const_iterator& operator --() { S_decrement(node); return *this; }
    const_iterator operator +(difference_type count) const { return const_iterator(*this) += count; }
    const_iterator operator -(difference_type count) const { return const_iterator(*this) -= count; }
    const_iterator operator ++(int) { const_iterator tmp = *this; ++tmp; return *this; }
    const_iterator operator --(int) { const_iterator tmp = *this; --tmp; return *this; }

    reference operator *() const { return node->value; }

    bool operator <(const_iterator const& other) const { return S_index(*this) < S_index(other); }
    bool operator ==(const_iterator const& other) const { return node == other.node; }
    bool operator <=(const_iterator const& other) const { return !(other < *this); }
    bool operator >(const_iterator const& other) const { return (other < *this); }
    bool operator >=(const_iterator const& other) const { return !(*this < other); }
    bool operator !=(const_iterator const& other) const { return !(*this == other); }
  };


private:
  static difference_type S_balance_factor(base_ptr node) {
    return S_balance_factor(std::const_pointer_cast<M_node const>(node));
  }
  static difference_type S_balance_factor(const_base_ptr node) {
    difference_type left = node->children[0]->height;
    difference_type right = node->children[1]->height;
    return left - right;
  }

  void M_prepare_sentinel() {
    assert(!M_root && !M_begin && !M_end);
    M_root = M_begin = M_end = base_ptr(new M_node);
  }

  void S_clear_dfs(base_ptr root) {
    for (auto child: root->children[0]) S_clear_dfs(child);
    delete root;
  }

  void M_clear() {
    M_size = 0;
    S_clear_dfs(M_root);
    M_begin = M_end = nullptr;
    M_prepare_sentinel();
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
      if (pos == pos->parent->chidlren[!dir]) {
        pos = pos->parent;
        return;
      }
      pos = pos->parent;
    }
  }

  static void S_advance(const_base_ptr& pos, difference_type count = +1) {
    if (i > 0) {
      while (pos->parent) {
        difference_type j = i + S_index_diff_to_parent(pos);
        if (j < 0) break;
        i = j;
        pos = pos->parent;
      }
    } else /* if (i < 0) */ {
      while (pos->parent) {
        difference_type j = i + S_index_diff_to_parent(pos);
        if (j > 0) break;
        i = j;
        pos = pos->parent;
      }
    }

    while (i != 0) {
      if (i > 0) {
        pos = pos->children[1];
        i -= pos->left_size + 1;
      } else /* if (i < 0) */ {
        pos = pos->children[0];
        i += pos->parent->left_size - pos->left_size;
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
    return S_index_diff_to_parent(std::const_base_ptr<M_node const>(pos));
  }
  static difference_type S_index_diff_to_parent(const_base_ptr const& pos) {
    if (pos == pos->parent->children[0]) {
      return -static_cast<difference_type>(pos->parent->left_size - pos->left_size);
    } else {
      return pos->left_size + 1;
    }
  }

  static const_base_ptr S_rotate(const_base_ptr pos, size_t dir, base_ptr& root) {
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
  }
  void M_rotate(const_base_ptr pos, size_t dir) { S_rotate(pos, dir, M_root); }

  const_base_ptr M_insert(const_base_ptr pos, const_base_ptr newnode) {
    if (pos->children[0]) {
      S_decrement(pos);
      pos->children[1] = newnode;
    } else {
      pos->children[0] = newnode;
      if (pos == M_begin) M_begin = std::const_pointer_cast<M_node>(pos);
    }
    newnode->parent = pos;
    ++M_size;
    M_fix_left_subtree_size(newnode, +1);
    M_insert_fix(newnode);
  }
  const_base_ptr M_erase(const_base_ptr pos) {
    if (pos->children[0] && pos->children[1]) {
      const_base_ptr tmp = pos;
      S_advance(tmp);
      pos->value = std::move(tmp->value);
      pos = tmp;
    }
    // maintain parent-child links appropriately
    // note that left sizes are also fixed

    // ascend from newnode to M_root
    // rotate appropriately if unbalanced node is found
  }

  const_base_ptr M_merge(const_base_ptr left, const_base_ptr right);
  // static const_base_ptr S_merge(const_base_ptr left, const_base_ptr right)
  const_base_ptr M_split(const_base_ptr pos);
  // static const_base_ptr S_split(const_base_ptr root, const_base_ptr pos)

public:
  avl_tree() { M_prepare_sentinel(); }
  avl_tree(avl_tree const&) = default;
  avl_tree(avl_tree&&) = default;
  template <typename InputIt>
  avl_tree(InputIt first, InputIt last) {
    M_prepare_sentinel();
    while (first != last) push_back(*first++);
  }
  avl_tree(std::initializer_list<value_type> ilist): avl_tree(ilist.begin(), ilist.end()) {}

  avl_tree& operator =(avl_tree const&) = default;
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
  reverse_iterator rbegin() { return M_end; }
  const_reverse_iterator rbegin() const { return M_end; }
  const_reverse_iterator crbegin() const { return M_end; }
  // ## rend
  reverse_iterator rend() { return M_begin; }
  const_reverse_iterator rend() const { return M_begin; }
  const_reverse_iterator crend() const { return M_begin; }

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
    return M_insert(pos.node, const_base_ptr(new M_node(value)));
  }
  iterator insert(const_iterator pos, value_type&& value) {
    return M_insert(pos.node, const_base_ptr(new M_node(std::move(value))));
  }
  iterator insert(const_iterator pos, size_t count, value_type const& value) {
    iterator res = pos;
    while (count--) res = M_insert(pos.node, const_base_ptr(new M_node(value)));
    return res;
  }
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    iterator res = pos;
    while (first != last) res = M_insert(pos.node, const_base_ptr(new M_node(*first++)));
    return res;
  }
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
    return insert(pos, ilist.begin(), ilist.end());
  }
  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return M_insert(pos.node, const_base_ptr(new M_node(std::forward<Args>(args)...)));
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
  void push_front(value_type&& value) const { insert(cbegin(), std::move(value)); }
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
  iterator merge(avl_tree&& other);
  avl_tree split(const_iterator pos);
};
