#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <memory>

template <typename Tp>
class rb_tree {
public:
  using value_type = Tp;
  using reference = value_type&;
  using const_reference = value_type const&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  class iterator;
  class const_iterator;

private:
  struct node_type;
  using pointer = std::shared_ptr<node_type>;
  using const_pointer = std::shared_ptr<node_type const>;

public:
  class iterator {
    friend rb_tree;
    friend const_iterator;
  public:
    using difference_type = typename rb_tree::difference_type;
    using value_type = typename rb_tree::value_type;
    using reference = typename rb_tree::reference;
    using const_reference = typename rb_tree::const_reference;
    using pointer = typename rb_tree::pointer;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer M_node;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(pointer ptr): M_node(ptr) {}
    explicit iterator(const_iterator const& other): M_node(std::const_pointer_cast<node_type>(other.M_node)) {}
    iterator& operator =(iterator const&) = default;
    iterator& operator =(iterator&&) = default;

    iterator& operator ++() { S_increment(M_node); return *this; }
    iterator operator ++(int) {
      iterator tmp = *this;
      ++*this;
      return tmp;
    }
    iterator& operator --() { S_decrement(M_node); return *this; }
    iterator operator --(int) {
      iterator tmp = *this;
      --*this;
      return tmp;
    }
    iterator& operator +=(difference_type i) { S_advance(M_node, i); return *this; }
    iterator operator +(difference_type i) const { return iterator(*this) += i; }
    iterator& operator -=(difference_type i) { S_advance(M_node, -i); return *this; }
    iterator operator -(difference_type i) const { return iterator(*this) -= i; }
    difference_type operator -(iterator const& other) const { return S_index(*this) - S_index(other); }

    reference operator *() { return M_node->value; }
    const_reference operator *() const { return M_node->value; }
    reference operator [](size_t i) { return *(*this + i); }
    const_reference operator [](size_t i) const { return *(*this + i); }

    bool operator ==(iterator const& other) const { return M_node == other.M_node; }
    bool operator !=(iterator const& other) const { return !(*this != other); }
    bool operator <(iterator const& other) const { return M_node < other.M_node; }
    // TODO comparison operations
  };

  class const_iterator {
    friend rb_tree;
    friend iterator;
  public:
    using difference_type = typename rb_tree::difference_type;
    using value_type = typename rb_tree::value_type const;
    using reference = typename rb_tree::const_reference;
    using const_reference = typename rb_tree::const_reference;
    using pointer = typename rb_tree::const_pointer;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer M_node;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator &&) = default;
    explicit const_iterator(iterator const& other): M_node(other.M_node) {}
    const_iterator& operator =(const_iterator const&) = default;
    const_iterator& operator =(const_iterator&&) = default;

    const_iterator& operator ++() { S_increment(M_node); return *this; }
    const_iterator operator ++(int) {
      const_iterator tmp = *this;
      ++*this;
      return tmp;
    }
    const_iterator& operator --() { S_decrement(M_node); return *this; }
    const_iterator operator --(int) {
      const_iterator tmp = *this;
      --*this;
      return tmp;
    }
    const_iterator& operator +=(difference_type i) { S_advance(M_node, i); return *this; }
    const_iterator operator +(difference_type i) const { return const_iterator(*this) += i; }
    const_iterator& operator -=(difference_type i) { S_advance(M_node, -i); return *this; }
    const_iterator operator -(difference_type i) const { return const_iterator(*this) -= i; }
    difference_type operator -(const_iterator const& other) const { return S_index(*this) - S_index(other); }

    const_reference operator *() const { return M_node->value; }
    const_reference operator [](size_t i) const { return *(*this + i); }

    bool operator ==(const_iterator const& other) const { return M_node == other.M_node; }
    bool operator !=(const_iterator const& other) const { return !(*this == other); }
    bool operator <(const_iterator const& other) const { return M_node < other.M_node; }
    // TODO comparison operations
  };

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  enum rb_tree_color { S_red = false, S_black = true };
  struct node_type {
    value_type value;
    std::shared_ptr<node_type> parent = nullptr;
    std::array<std::shared_ptr<node_type>, 2> children = {nullptr, nullptr};
    rb_tree_color color = S_red;
    size_type left_size = 0;
    node_type() = default;
    node_type(value_type const& x): value(x) {}
    node_type(value_type&& x): value(x) {}
  };

  void M_deep_copy(rb_tree& dst, rb_tree const& src);
  void M_release();
  void M_clear();

  static difference_type S_index_diff_to_parent(const_pointer const& pos) {
    return S_index_diff_to_parent(std::const_pointer_cast<node_type>(pos));
  }
  static difference_type S_index_diff_to_parent(pointer const& pos) {
    if (pos == pos->parent->children[0]) {
      return -static_cast<difference_type>(pos->parent->left_size - pos->left_size);
    } else {
      return pos->left_size + 1;
    }
  }

  static void S_increment(const_pointer& pos, size_type dir = 1) {
    if (pos->children[dir]) {
      pos = pos->children[dir];
      while (pos->children[!dir]) pos = pos->children[!dir];
      return;
    }
    while (true) {
      // if (!pos->parent) {
      //   if (dir) throw std::out_of_range("attempt to increment a past-the-end iterator");
      //   throw std::out_of_range("attempt to decrement start-of-sequence iterator");
      // }
      if (pos == pos->parent->children[!dir]) {
        pos = pos->parent;
        return;
      }
      pos = pos->parent;
    }
  }
  static void S_increment(pointer& pos, size_type dir = 1) {
    fprintf(stderr, "%scrementing %zu-th iterator\n", dir? "in":"de", S_index(pos));
    if (pos->children[dir]) {
      pos = pos->children[dir];
      while (pos->children[!dir]) pos = pos->children[!dir];
      return;
    }
    while (true) {
      if (!pos->parent) {
        if (dir) throw std::out_of_range("attempt to increment a past-the-end iterator");
        throw std::out_of_range("attempt to decrement start-of-sequence iterator");
      }
      if (pos == pos->parent->children[!dir]) {
        pos = pos->parent;
        return;
      }
      pos = pos->parent;
    }
  }

  static void S_decrement(const_pointer& pos) { S_increment(pos, 0); }
  static void S_decrement(pointer& pos) { S_increment(pos, 0); }

  static void S_advance(pointer& pos, difference_type i = +1) {
    if (i > 0) {
      while (pos->parent) {
        intmax_t j = i + S_index_diff_to_parent(pos);
        if (j < 0) break;
        i = j;
        pos = pos->parent;
      }
    } else /* if (i < 0) */ {
      while (pos->parent) {
        intmax_t j = i + S_index_diff_to_parent(pos);
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
  static void S_advance(const_pointer& pos, difference_type i = +1) {
    if (i > 0) {
      while (pos->parent) {
        intmax_t j = i + S_index_diff_to_parent(pos);
        if (j < 0) break;
        i = j;
        pos = pos->parent;
      }
    } else /* if (i < 0) */ {
      while (pos->parent) {
        intmax_t j = i + S_index_diff_to_parent(pos);
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

  static bool S_is_red(pointer const& pos) { return pos && pos->color == S_red; }
  static bool S_is_red(const_pointer const& pos) { return pos && pos->color == S_red; }

  void M_rotate(pointer cur, size_type dir) {
    pointer child = cur->children[!dir];
    cur->children[!dir] = child->children[dir];
    if (child->children[dir])
      child->children[dir]->parent = cur;

    child->parent = cur->parent;
    if (!cur->parent) {
      M_root = child;
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

  pointer M_insert(const_iterator pos_it, pointer newnode) {
    return M_insert(iterator(pos_it), newnode);
  }

  pointer M_insert(iterator pos_it, pointer newnode) {
    assert(newnode);
    pointer pos = pos_it.M_node;

    newnode->children[0] = newnode->children[1] = nullptr;
    newnode->left_size = 0;
    newnode->color = S_red;

    if (pos->children[0]) {
      S_decrement(pos);
      pos->children[1] = newnode;
    } else {
      pos->children[0] = newnode;
      if (pos == M_begin) M_begin = newnode;
    }
    newnode->parent = pos;
    ++M_size;
    M_fix_left_subtree_size(newnode, +1);
    M_insert_fix(newnode);
    return newnode;
  }

  void M_insert_fix(pointer pos) {
    while (S_is_red(pos->parent)) {
      pointer gparent(pos->parent->parent);
      size_type uncle_dir = (pos->parent != gparent->children[1]);
      pointer uncle(gparent->children[uncle_dir]);
      if (S_is_red(uncle)) {
        pos->parent->color = uncle->color = S_black;
        gparent->color = S_red;
        pos = gparent;
        continue;
      }
      if (pos == pos->parent->children[uncle_dir]) {
        pos = pos->parent;
        M_rotate(pos, !uncle_dir);
      }
      pos->parent->color = S_black;
      pos->parent->parent->color = S_red;
      M_rotate(gparent, uncle_dir);
    }
    M_root->color = S_black;
  }

  pointer M_erase(const_iterator pos_it) {
    return M_erase(iterator(pos_it));
  }

  pointer M_erase(iterator pos_it) {
    pointer pos = pos_it.M_node;
    pointer res = std::next(pos_it).M_node;
    if (pos == M_begin) M_begin = res;

    --M_size;
    pointer y = pos;
    if (pos->children[0] && pos->children[1]) {
      pos->value = std::move(res->value);
      S_increment(y);
      S_increment(res);
    }

    pointer x = y->children[0];
    if (!x) x = y->children[1];
    if (x) x->parent = y->parent;
    if (!y->parent) {
      M_root = x;
    } else {
      M_fix_left_subtree_size(y, -1);
      size_type ydir = (y == y->parent->children[1]);
      y->parent->children[ydir] = x;
    }
    pointer xparent = y->parent;  // x may be nil
    bool fix_needed = (y->color == S_black);
    if (fix_needed) M_erase_fix(x, xparent);
    return res;
  }
  void M_erase_fix(pointer pos, pointer parent) {
    while (pos != M_root && !S_is_red(pos)) {
      size_type sibling_dir = (pos == parent->children[0]);
      pointer sibling = parent->children[sibling_dir];

      if (S_is_red(sibling)) {
        sibling->color = S_black;
        parent->color = S_red;
        M_rotate(parent, !sibling_dir);
        sibling = parent->children[sibling_dir];
      }
      if (sibling) {
        if (!S_is_red(sibling->children[0]) && !S_is_red(sibling->children[1])) {
          sibling->color = S_red;
          pos = parent;
          parent = pos->parent;
          continue;
        }

        if (!S_is_red(sibling->children[sibling_dir])) {
          if (sibling->children[!sibling_dir])
            sibling->children[!sibling_dir]->color = S_black;

          sibling->color = S_red;
          M_rotate(sibling, sibling_dir);
          sibling = parent->children[sibling_dir];
        }

        sibling->color = S_black;
        sibling->children[sibling_dir]->color = S_black;
      }

      parent->color = S_black;
      M_rotate(parent, !sibling_dir);
      pos = M_root;
      parent = nullptr;
    }
    if (pos) pos->color = S_black;
  }

  // M_merge();
  // M_split();

  void M_calculate_size(const_iterator subroot) const;
  void M_fix_left_subtree_size(pointer cur, difference_type diff) {
    while (cur->parent) {
      if (cur == cur->parent->children[0])
        cur->parent->left_size += diff;
      cur = cur->parent;
    }
  }

  static size_type S_index(iterator const& pos) { return S_index(const_iterator(pos)); }
  static size_type S_index(const_iterator const& pos) {
    // if (pos == cbegin()) return 0;
    // if (pos == cend()) return M_size;
    size_type res = 0;
    const_pointer cur = pos.M_node;
    while (cur->parent) {
      if (cur == cur->parent->children[1])
        res += cur->parent->left_size + 1;
      cur = cur->parent;
    }
    return res;
  }

  void M_inspect_dfs(const const_pointer& root, size_type depth = 0) const {
    auto child = root->children[1];
      if (child) M_inspect_dfs(child, depth+1);
    fprintf(stderr, "%*s-(%s%p%s) (%zu)\n",
            static_cast<int>(depth), "",
            S_is_red(root)? "\x1b[1;31m":"\x1b[1m",
            root.get(),
            "\x1b[m",
            root->left_size
            );
    child = root->children[0];
      if (child) M_inspect_dfs(child, depth+1);
  }

  pointer M_begin, M_end, M_root;
  size_type M_size = 0;

public:
  rb_tree() {
    M_root = M_begin = M_end = pointer(new node_type);
    M_root->color = S_black;
  }
  rb_tree(rb_tree&&) = default;
  rb_tree(rb_tree const& other) { M_deep_copy(*this, other); }
  rb_tree(size_type n, value_type const& x = value_type{}): rb_tree() {
    for (size_t i = 0; i < n; ++i) push_back(x);
  }
  rb_tree(std::initializer_list<value_type> const& ilist): rb_tree() {
    for (auto& x: ilist) push_back(x);
  }
  template <typename InputIt>
  rb_tree(InputIt first, InputIt last): rb_tree() {
    while (first != last) push_back(*first++);
  }

  rb_tree& operator =(rb_tree&&) = default;
  rb_tree& operator =(rb_tree const& other) { M_deep_copy(*this, other); }

  // insertions and an emplacement
  iterator insert(const_iterator pos, value_type const& value) {
    return M_insert(pos, pointer(new node_type(value)));
  }
  iterator insert(const_iterator pos, value_type&& value) {
    return M_insert(pos, pointer(new node_type(value)));
  }
  iterator insert(const_iterator pos, size_type count, value_type const& value) {
    iterator res;
    for (size_type i = 0; i < count; ++i)
      res = M_insert(pos, pointer(new node_type(value)));
    return res;
  }
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    iterator res;
    while (first != last)
      res = M_insert(pos, pointer(new node_type(*first++)));
    return res;
  }
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
    iterator res;
    for (auto const& x: ilist)
      res = M_insert(pos, pointer(new node_type(x)));
    return res;
  }
  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return M_insert(pos, pointer(new node_type(std::forward<Args>(args)...)));
  }
  iterator push_back(value_type const& value) { return insert(cend(), value); }
  iterator push_back(value_type&& value) { return insert(cend(), value); }
  iterator push_front(value_type const& value) { return insert(cbegin(), value); }
  iterator push_front(value_type&& value) { return insert(cbegin(), value); }

  // erasures
  iterator erase(iterator pos) { return M_erase(pos); }
  iterator erase(const_iterator pos) { return M_erase(pos); }
  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) first = erase(first);
    return first;
  }
  iterator pop_front() { return erase(begin()); }
  iterator pop_back() { return erase(--end()); }

  // merge and split
  iterator merge(rb_tree&& other) { return M_merge(other); }
  rb_tree split(iterator pos) { return M_split(pos); }
  rb_tree split(const_iterator pos) { return M_split(pos); }

  // other modifier(s)
  void clear() { M_clear(); }

  // boundaries
  template <typename Comparator>
  const_iterator lower_bound(value_type const& key, Comparator comp) const;
  template <typename Comparator>
  const_iterator upper_bound(value_type const& key, Comparator comp) const;

  // basic iterators
  iterator begin() { return iterator(M_begin); }
  const_iterator begin() const { return const_iterator(M_begin); }
  const_iterator cbegin() const { return const_iterator(M_begin); }
  iterator end() { return iterator(M_end); }
  const_iterator end() const { return const_iterator(M_end); }
  const_iterator cend() const { return const_iterator(M_end); }
  reverse_iterator rbegin() { return reverse_iterator(M_end); }
  const_reverse_iterator rbegin() const { return reverse_iterator(M_end); }
  const_reverse_iterator crbegin() const { return const_reverse_iterator(M_end); }
  reverse_iterator rend() { return reverse_iterator(M_begin); }
  const_reverse_iterator rend() const { return const_reverse_iterator(M_begin); }
  const_reverse_iterator crend() const { return const_reverse_iterator(M_begin); }

  // capacities
  bool empty() const { return (M_size == 0); }
  size_type size() const { return M_size; }

  // accessors
  size_type index(iterator pos) const { return S_index(const_iterator(pos)); }
  size_type index(const_iterator pos) const { return S_index(pos); }
  reference operator [](size_type pos) { return begin()[pos]; }
  const_reference operator [](size_type pos) const { return cbegin()[pos]; }
  // the i-th iterator should be obtained via begin()+i
  reference at(size_type pos) {
    if (pos >= M_size) throw std::out_of_range("n >= size");
    return begin()[pos];
  }
  const_reference at(size_type pos) const {
    if (pos >= M_size) throw std::out_of_range("n >= size");
    return cbegin()[pos];
  }
  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *rbegin(); }
  const_reference back() const { return *rbegin(); }

  void inspect() const {
    fprintf(stderr, "root: %p\n", M_root.get());
    fprintf(stderr, "size: %zu\n", M_size);
    M_inspect_dfs(std::const_pointer_cast<node_type const>(M_root));
  }
};

#include <numeric>
#include <random>

int main() {
  {
    rb_tree<int> rbt;
    rbt.inspect();
    rbt.push_front(1);
    rbt.inspect();
    rbt.push_back(2); 
    rbt.inspect();
    rbt.pop_front();
    rbt.inspect();
    int x = *rbt.begin();
    printf("%d\n", x);
    printf("%zu\n", rbt.index(rbt.end()));
  }

  if (true) {
    int n = 16;
    std::vector<int> test(n);
    std::iota(test.begin(), test.end(), 0);
    rb_tree<int> rbt(test.begin(), test.end());
    // rbt.inspect();

    // auto it = rbt.begin();
    // for (int i = 0; i < n; ++i) {
    //   for (int j = 0; j < n; ++j) {
    //     int x = *(it+(j-i));
    //     int y = *(it-(i-j));
    //     fprintf(stderr, "(%d, %d): %d, %d\n", i, j, x, y);
    //     assert(x == j && y == j);
    //   }
    //   ++it;
    // }

    for (int i = 0; i < n; ++i) {
      fprintf(stderr, "%d\n", rbt[i]);
      assert(rbt[i] == i);
    }

    std::mt19937 rsk(0315);
    std::shuffle(rbt.begin(), rbt.end(), rsk);
    fprintf(stderr, "shuffled\n");
    for (int i = 0; i < n; ++i) fprintf(stderr, "%d\n", rbt[i]);

    std::sort(rbt.begin(), rbt.end());
    fprintf(stderr, "sorted\n");
    for (int i = 0; i < n; ++i) fprintf(stderr, "%d\n", rbt[i]);

    fprintf(stderr, ".begin(): %zu\n", rbt.index(rbt.begin()));
    fprintf(stderr, ".end(): %zu\n", rbt.index(rbt.end()));
  }

  if (true) {
    rb_tree<int> rbt{1, 2, 3};
    for (size_t i = 0; i < 3; ++i) printf("%d\n", rbt[i]);
    rbt[1] = 4;
    (rbt.begin()[1]) = 3;
    *(rbt.begin()+1) = 2;
    for (size_t i = 0; i < 3; ++i) printf("%d\n", rbt[i]);
  }
}
