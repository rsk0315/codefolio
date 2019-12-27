#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <memory>
#include <array>

constexpr intmax_t  operator ""_jd(unsigned long long n) { return n; }
constexpr uintmax_t operator ""_ju(unsigned long long n) { return n; }
constexpr size_t    operator ""_zu(unsigned long long n) { return n; }
// constexpr ptrdiff_t operator ""_td(unsigned long long n) { return n; }

template <typename Tp>
class order_statistic_tree {
  // based on splay tree

public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;
  class iterator;
  class const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  class node;
  using pointer = std::shared_ptr<node>;

  class node {
    friend order_statistic_tree;
    friend iterator;
    friend const_iterator;

  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp&;
    using const_reference = Tp const&;

  private:
    using pointer = std::shared_ptr<node>;

    mutable pointer M_parent = nullptr;
    mutable std::array<pointer, 2> M_children{{nullptr, nullptr}};
    value_type M_value;
    mutable size_type M_size = 1;

  public:
    node() = default;
    node(node const&) = delete;  // !
    node(node&&) = default;
    node(const_reference value): M_value(value) {}
    node(value_type&& value): M_value(std::move(value)) {}

    ~node() {
      // if (M_children[0]) delete M_children[0].get();
      // if (M_children[1]) delete M_children[1].get();
    }

    void set_size() const {
      // for splaying
      M_size = 1;
      if (M_children[0]) M_size += M_children[0]->M_size;
      if (M_children[1]) M_size += M_children[1]->M_size;
    }

    size_type left_size() const {
      if (!M_children[0]) return 0;
      return M_children[0]->M_size;
    }
    size_type right_size() const {
      if (!M_children[1]) return 0;
      return M_children[1]->M_size;
    }
  };

  static pointer S_next(pointer ptr, order_statistic_tree* tree,
                        difference_type n = 1) {

    if (n == 0) return ptr;
    if (n < 0) return S_prev(ptr, tree, -n);

    tree->M_root = S_splay(ptr);
    // fprintf(stderr, "null? %d\n", !ptr);
    // if (ptr) fprintf(stderr, "next: %p, size: %zu\n", ptr.get(), ptr->M_size);
    ptr = ptr->M_children[1];
    --n;
    while (ptr) {
      difference_type ls = ptr->left_size();
      if (n == ls) return S_splay(ptr);
      if (n < ls) {
        ptr = ptr->M_children[0];
      } else {
        ptr = ptr->M_children[1];
        n -= ls+1;
      }
    }
    return nullptr;
  }
  static pointer S_prev(pointer ptr, order_statistic_tree* tree,
                        difference_type n = 1) {

    if (n == 0) return S_splay(ptr);
    if (n < 0) return S_next(ptr, tree, -n);
    if (!ptr)
      return S_prev(tree->M_rightmost, tree, n-1);  // past-the-end

    tree->M_root = S_splay(ptr);
    ptr = ptr->M_children[0];
    --n;
    while (ptr) {
      difference_type rs = ptr->right_size();
      if (n == rs) return S_splay(ptr);
      if (n < rs) {
        ptr = ptr->M_children[1];
      } else {
        ptr = ptr->M_children[0];
        n -= rs+1;
      }
    }
    return nullptr;
  }

  static size_type S_index(pointer ptr, order_statistic_tree* tree) {
    if (!ptr) return tree->size();
    tree->M_root = S_splay(ptr);
    return ptr->left_size();
  }

  static size_type S_parent_dir(pointer ptr) {
    if (!ptr->M_parent) return -1_zu;
    if (ptr->M_parent->M_children[0] == ptr) return 0;
    if (ptr->M_parent->M_children[1] == ptr) return 1;
    return -1_zu;
  }

  static pointer S_zig(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer sub = cur->M_children[!dir];

    cur->M_parent = nullptr;\
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub;
    if (sub) sub->M_parent = par;

    par->set_size();
    cur->set_size();
    return cur;
  }
  static pointer S_zigzig(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[!dir];
    pointer sub2 = par->M_children[!dir];
    size_type gd = S_parent_dir(gpar);

    cur->M_parent = gpar->M_parent;
    if (gd != -1_zu) cur->M_parent->M_children[gd] = cur;
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub1;
    if (sub1) sub1->M_parent = par;
    par->M_children[!dir] = gpar;
    gpar->M_parent = par;
    gpar->M_children[dir] = sub2;
    if (sub2) sub2->M_parent = gpar;

    gpar->set_size();
    par->set_size();
    cur->set_size();
    return cur;
  }
  static pointer S_zigzag(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[dir];
    pointer sub2 = cur->M_children[!dir];
    size_type gd = S_parent_dir(gpar);

    cur->M_parent = gpar->M_parent;
    if (gd != -1_zu) cur->M_parent->M_children[gd] = cur;
    cur->M_children[dir] = gpar;
    gpar->M_parent = cur;
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    gpar->M_children[!dir] = sub1;
    if (sub1) sub1->M_parent = gpar;
    par->M_children[dir] = sub2;
    if (sub2) sub2->M_parent = par;

    gpar->set_size();
    par->set_size();
    cur->set_size();
    return cur;
  }

  static pointer S_splay(pointer ptr) {
    if (!ptr) return nullptr;
    while (true) {
      size_type pd = S_parent_dir(ptr);
      if (pd == -1_zu) return ptr;
      size_type gd = S_parent_dir(ptr->M_parent);
      if (gd == -1_zu) return S_zig(ptr, pd);
      ptr = ((gd == pd)? S_zigzig(ptr, pd): S_zigzag(ptr, pd));
    }
  }

public:
  class iterator {
    friend order_statistic_tree;
    friend const_iterator;

  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp&;
    using const_reference = Tp const&;
    using pointer = std::shared_ptr<node>;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer M_ptr = nullptr;
    mutable order_statistic_tree* M_tree = nullptr;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(pointer ptr, order_statistic_tree* tree):
      M_ptr(ptr), M_tree(tree) {}

    iterator& operator =(iterator const&) = default;
    iterator& operator =(iterator&&) = default;

    iterator& operator ++() { M_ptr = S_next(M_ptr, M_tree); return *this; }
    iterator& operator --() { M_ptr = S_prev(M_ptr, M_tree); return *this; }

    iterator& operator +=(difference_type n) {
      M_ptr = S_next(M_ptr, M_tree, n); return *this;
    }
    iterator& operator -=(difference_type n) {
      M_ptr = S_prev(M_ptr, M_tree, n); return *this;
    }

    iterator operator ++(int) { iterator tmp(*this); ++*this; return tmp; }
    iterator operator --(int) { iterator tmp(*this); --*this; return tmp; }

    iterator operator +(difference_type n) const {
      return iterator(*this) += n;
    }
    iterator operator -(difference_type n) const {
      return iterator(*this) -= n;
    }

    difference_type operator -(const_iterator const& other) const {
      return S_index(M_ptr, M_tree) - S_index(other.M_ptr, M_tree);
    }

    bool operator ==(const_iterator const& other) const {
      return M_ptr == other.M_ptr;
    }
    bool operator !=(const_iterator const& other) const {
      return !(*this == other);
    }

    bool operator <(const_iterator const& other) const {
      return S_index(M_ptr, M_tree) < S_index(other.M_ptr, M_tree);
    }
    bool operator >(const_iterator const& other) const {
      return other < *this;
    }
    bool operator <=(const_iterator const& other) const {
      return !(other < *this);
    }
    bool operator >=(const_iterator const& other) const {
      return !(*this < other);
    }

    reference operator *() {
      M_tree->M_root = S_splay(M_ptr); return M_ptr->M_value;
    }
    const_reference operator *() const {
      M_tree->M_root = S_splay(M_ptr); return M_ptr->M_value;
    }
  };

public:
  class const_iterator {
    friend order_statistic_tree;
    friend iterator;

  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp const&;
    using const_reference = Tp const&;
    using pointer = std::shared_ptr<node>;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer M_ptr = nullptr;
    mutable order_statistic_tree* M_tree = nullptr;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    const_iterator(iterator const& other): M_ptr(other.M_ptr) {}
    const_iterator(iterator&& other): M_ptr(other.M_ptr) {}
    const_iterator(pointer ptr, order_statistic_tree* tree):
      M_ptr(ptr), M_tree(tree) {}

    const_iterator& operator =(const_iterator const&) = default;
    const_iterator& operator =(const_iterator&&) = default;
    const_iterator& operator =(iterator const& other) {
      M_ptr = other.M_ptr;
      M_tree = other.M_tree;
      return *this;
    }
    const_iterator& operator =(iterator&& other) {
      M_ptr = other.M_ptr;
      M_tree = other.M_tree;
      return *this;
    }

    const_iterator& operator ++() { M_ptr = S_next(M_ptr, M_tree); return *this; }
    const_iterator& operator --() { M_ptr = S_prev(M_ptr, M_tree); return *this; }

    const_iterator& operator +=(difference_type n) {
      M_ptr = S_next(M_ptr, M_tree, n); return *this;
    }
    const_iterator& operator -=(difference_type n) {
      M_ptr = S_prev(M_ptr, M_tree, n); return *this;
    }

    const_iterator operator ++(int) {
      const_iterator tmp(*this); ++*this; return tmp;
    }
    const_iterator operator --(int) {
      const_iterator tmp(*this); --*this; return tmp;
    }

    const_iterator operator +(difference_type n) const {
      return const_iterator(*this) += n;
    }
    const_iterator operator -(difference_type n) const {
      return const_iterator(*this) -= n;
    }

    difference_type operator -(const_iterator const& other) const {
      return S_index(M_ptr, M_tree) - S_index(other.M_ptr, M_tree);
    }

    bool operator ==(const_iterator const& other) const {
      return M_ptr == other.M_ptr;
    }
    bool operator !=(const_iterator const& other) const {
      return !(*this == other);
    }

    bool operator <(const_iterator const& other) const {
      return S_index(M_ptr, M_tree) < S_index(other.M_ptr, M_tree);
    }
    bool operator >(const_iterator const& other) const {
      return other < *this;
    }
    bool operator <=(const_iterator const& other) const {
      return !(other < *this);
    }
    bool operator >=(const_iterator const& other) const {
      return !(*this < other);
    }

    reference operator *() const {
      M_tree->M_root = S_splay(M_ptr); return M_ptr->M_value;
    }
  };

private:
  size_type M_size = 0;
  pointer M_root = nullptr;
  pointer M_leftmost = nullptr;
  pointer M_rightmost = nullptr;

  static pointer S_leftmost(pointer root) {
    if (!root) return nullptr;
    while (root->M_children[0]) root = root->M_children[0];
    return root;
  }
  static pointer S_rightmost(pointer root) {
    if (!root) return nullptr;
    while (root->M_children[1]) root = root->M_children[1];
    return root;
  }

  void M_init_ends() {
    M_leftmost = M_rightmost = M_root;
    if (!M_root) return;
    M_leftmost = S_leftmost(M_root);
    M_rightmost = S_rightmost(M_root);
  }

  static void S_deep_copy(pointer& dst, pointer const& src) {
    dst = std::make_shared<node>(src->M_value);
    for (size_type i = 0; i <= 1; ++i)
      if (src->M_children[i])
        S_deep_copy(dst->M_children[i], src->M_children[i]);
  }

  void M_deep_copy(order_statistic_tree const& other) {
    clear();
    if (!other.M_root) return;

    S_deep_copy(M_root, other.M_root);
    M_size = other.M_size;
    M_init_ends();
  }

  void M_move(order_statistic_tree&& other) {
    clear();
    M_root = other.M_root;
    M_leftmost = other.M_leftmost;
    M_rightmost = other.M_rightmost;
    M_size = other.M_size;
    other.clear();
  }

  // explicit order_statistic_tree(pointer root): M_root(root) {
  //   if (!root) return;
  //   M_root->set_size();
  //   M_size = M_root->M_size;
  //   M_init_ends();
  // }

public:
  order_statistic_tree() = default;
  order_statistic_tree(order_statistic_tree const& other) {
    M_deep_copy(other);
  }
  order_statistic_tree(order_statistic_tree&& other) {
    M_move(std::move(other));
  }

  template <typename InputIt>
  order_statistic_tree(InputIt first, InputIt last) { assign(first, last); }
  order_statistic_tree(std::initializer_list<value_type> il) { assign(il); }

  explicit order_statistic_tree(size_type size, const_reference value) {
    assign(size, value);
  }
  explicit order_statistic_tree(size_type size) { assign(size); }

  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    clear();
    while (first != last) {
      pointer tmp = M_root;
      M_root = std::make_shared<node>(*first++);
      M_root->M_children[0] = tmp;
      M_root->M_size = ++M_size;
      if (tmp) tmp->M_parent = M_root;
    }
    M_init_ends();
  }
  void assign(std::initializer_list<value_type> il) {
    assign(il.begin(), il.end());
  }

  void assign(size_type size, const_reference value) {
    clear();
    for (size_type i = 0; i < size; ++i) {
      pointer tmp = M_root;
      M_root = std::make_shared<node>(value);
      M_root->M_children[0] = tmp;
      M_root->M_size = ++M_size;
      if (tmp) tmp->M_parent = M_root;
    }
    M_init_ends();
  }
  void assign(size_type size) {
    clear();
    for (size_type i = 0; i < size; ++i) {
      pointer tmp = M_root;
      M_root = std::make_shared<node>();
      M_root->M_children[0] = tmp;
      M_root->M_size = ++M_size;
      if (tmp) tmp->M_parent = M_root;
    }
    M_init_ends();
  }

  void clear() {
    M_size = 0;
    M_root = M_leftmost = M_rightmost = nullptr;
  }

  order_statistic_tree& operator =(order_statistic_tree const& other) {
    M_deep_copy(other);
    return *this;
  }
  order_statistic_tree& operator =(order_statistic_tree&& other) {
    M_move(std::move(other));
    return *this;
  }

  reference at(size_type n) {
    if (n >= size()) throw std::out_of_range("n >= size()");
    return *(begin() + n);
  }
  const_reference at(size_type n) const {
    if (n >= size()) throw std::out_of_range("n >= size()");
    return *(begin() + n);
  }

  reference operator [](size_type n) { return *(begin() + n); }
  const_reference operator [](size_type n) const { return *(begin() + n); }

  reference front() { return *begin(); }
  const_reference front() const { *begin(); }
  reference back() { return *rbegin(); }
  const_reference back() const { *rbegin(); }

  iterator begin() { return iterator(M_leftmost, this); }
  const_reference begin() const { return const_reference(M_leftmost, this); }
  const_reference cbegin() const { return const_reference(M_leftmost, this); }

  iterator end() { return iterator(nullptr, this); }
  const_reference end() const { return const_reference(nullptr, this); }
  const_reference cend() const { return const_reference(nullptr, this); }

  reverse_iterator rbegin() { return end(); }
  const_reverse_iterator rbegin() const { return end(); }
  const_reverse_iterator crbegin() const { return cend(); }

  reverse_iterator rend() { return begin(); }
  const_reverse_iterator rend() const { return begin(); }
  const_reverse_iterator crend() const { return cbegin(); }

  bool empty() const noexcept { return M_size == 0; }
  size_type size() const { return M_size; }

  iterator insert(const_iterator pos, const_reference value) {
    // fprintf(stderr, "pos: %p\n", pos.M_ptr.get());
    if (pos == end()) return push_back(value);
    M_root = S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    // fprintf(stderr, "split [%zu]:[%zu]\n", size(), tmp.size());
    iterator res = push_back(value);
    // fprintf(stderr, "inserted [%zu]:[%zu]\n", size(), tmp.size());
    merge(std::move(tmp));
    // fprintf(stderr, "begin(): %p\n", begin().M_ptr.get());
    return res;
  }

  iterator insert(const_iterator pos, value_type&& value) {
    if (pos == end()) return push_back(std::move(value));
    M_root = S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(std::move(value));
    merge(std::move(tmp));
    return res;
  }

  iterator insert(const_iterator pos, size_type count, const_reference value) {
    if (count == 0) return pos;
    if (pos == end()) {
      auto res = push_back(value);
      for (size_type i = 1; i < count; ++i) push_back(value);
      return res;
    }
    M_root = S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(value);
    for (size_type i = 1; i < count; ++i) push_back(value);
    merge(std::move(tmp));
    return res;
  }

  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    if (first == last) return pos;
    if (pos == end()) {
      auto res = push_back(*first++);
      while (first != last) push_back(*first++);
      return res;
    }
    M_root = S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(*first++);
    while (first != last) push_back(*first++);
    merge(std::move(tmp));
    return res;
  }

  iterator insert(const_iterator pos, std::initializer_list<value_type> il) {
    return insert(pos, il.begin(), il.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    if (pos == end()) return emplace_back(std::forward<Args>(args)...);
    M_root = S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = emplace_back(std::forward<Args>(args)...);
    merge(std::move(tmp));
    return res;    
  }

  iterator push_back(const_reference value) {
    M_root = S_splay(M_rightmost);
    pointer tmp = std::make_shared<node>(value);
    tmp->M_children[0] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_leftmost) M_leftmost = tmp;
    M_root = M_rightmost = tmp;
    return iterator(tmp, this);
  }

  iterator push_back(value_type&& value) {
    M_root = S_splay(M_rightmost);
    pointer tmp = std::make_shared<node>(std::move(value));
    tmp->M_children[0] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_leftmost) M_leftmost = tmp;
    M_root = M_rightmost = tmp;
    return iterator(tmp, this);
  }

  template <typename... Args>
  iterator emplace_back(Args&&... args) {
    M_root = S_splay(M_rightmost);
    pointer tmp = std::make_shared<node>(std::forward<Args>(args)...);
    tmp->M_children[0] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_leftmost) M_leftmost = tmp;
    M_root = M_rightmost = tmp;
    return iterator(tmp, this);
  }

  iterator push_front(const_reference value) {
    M_root = S_splay(M_leftmost);
    pointer tmp = std::make_shared<node>(value);
    tmp->M_children[1] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_rightmost) M_rightmost = tmp;
    M_root = M_leftmost = tmp;
    return iterator(tmp, this);
  }

  iterator push_front(value_type&& value) {
    M_root = S_splay(M_leftmost);
    pointer tmp = std::make_shared<node>(std::move(value));
    tmp->M_children[1] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_rightmost) M_rightmost = tmp;
    M_root = M_leftmost = tmp;
    return iterator(tmp, this);
  }

  template <typename... Args>
  iterator emplace_front(Args&&... args) {
    M_root = S_splay(M_leftmost);
    pointer tmp = std::make_shared<node>(std::forward<Args>(args)...);
    tmp->M_children[1] = M_root;
    tmp->M_size = ++M_size;
    if (M_root) M_root->M_parent = tmp;
    if (!M_rightmost) M_rightmost = tmp;
    M_root = M_leftmost = tmp;
    return iterator(tmp, this);
  }

  iterator pop_back() {
    M_root = S_splay(M_rightmost);
    M_root = M_rightmost->M_children[0];
    if (M_root) M_root->M_parent = nullptr;
    M_rightmost->M_children[0] = nullptr;
    M_rightmost = S_rightmost(M_root);
    if (--M_size == 0) M_leftmost = nullptr;
    return end();
  }

  iterator pop_front() {
    M_root = S_splay(M_leftmost);
    M_root = M_leftmost->M_children[1];
    if (M_root) M_root->M_parent = nullptr;
    M_leftmost->M_children[1] = nullptr;
    M_leftmost = S_leftmost(M_root);
    if (--M_size == 0) M_rightmost = nullptr;
    return begin();
  }

  iterator erase(const_iterator pos) {
    if (pos == begin()) return pop_front();
    order_statistic_tree tmp = split(pos);
    tmp.pop_front();
    // fprintf(stderr, "tmp.root: %d\n", tmp.M_root->M_value);
    // fprintf(stderr, "size: %zu, %zu\n", size(), tmp.size());
    auto res = tmp.begin();
    merge(std::move(tmp));
    return res;
  }

  iterator erase(const_iterator first, const_iterator last) {
    if (first == last) return first;
    order_statistic_tree tmp = split(last);
    split(first);  // discard
    auto res = tmp.begin();
    merge(std::move(tmp));
    return res;
  }

  iterator merge(order_statistic_tree& other) {
    return merge(std::move(other));
  }

  iterator merge(order_statistic_tree&& other) {
    if (other.empty()) return end();
    if (empty()) {
      M_move(std::move(other));
      return begin();
    }
    M_root = S_splay(M_rightmost);
    M_root->M_children[1] = other.M_root;
    other.M_root->M_parent = M_root;
    M_size = (M_root->M_size += other.M_size);
    M_rightmost = other.M_rightmost;
    iterator res(other.M_leftmost, this);
    other.clear();  // not destruct nodes
    // fprintf(stderr, "[%p] [%p]\n", M_root->M_children[0].get(), M_root->M_children[1].get());
    return res;
  }

  order_statistic_tree split(const_iterator pos) {
    if (pos == end()) return {};
    if (pos == begin()) return std::move(*this);
    M_root = S_splay(pos.M_ptr);
    // fprintf(stderr, "split: %p %zu\n", M_root.get(), M_root->M_size);
    pointer resp = M_root;
    M_root = resp->M_children[0];
    resp->M_children[0] = nullptr;
    resp->set_size();
    if (M_root) {
      M_root->M_parent = nullptr;
      M_root->set_size();
      M_size = M_root->M_size;
    } else {
      M_size = 0;
    }

    order_statistic_tree res;
    res.M_root = resp;
    res.M_size = resp->M_size;
    res.M_leftmost = pos.M_ptr;
    res.M_rightmost = M_rightmost;

    M_rightmost = S_rightmost(M_root);
    // if (M_leftmost) fprintf(stderr, "leftmost: %d\n", M_leftmost->M_value);
    // if (M_rightmost) fprintf(stderr, "rightmost: %d\n", M_rightmost->M_value);
    // if (res.M_leftmost) fprintf(stderr, "other.leftmost: %d\n", res.M_leftmost->M_value);
    // if (res.M_rightmost) fprintf(stderr, "other.rightmost: %d\n", res.M_rightmost->M_value);
    return res;
  }

  template <typename Predicate>
  iterator partition_point(Predicate pred) {
    pointer cur = M_root;
    if (!cur) return end();
    // ... true false ...
    //          ^
    // fprintf(stderr, "root: %d\n", cur->M_value);
    // fprintf(stderr, "left: [%p], right: [%p]\n", cur->M_children[0].get(), cur->M_children[1].get());
    while (true) {
      pointer tmp = cur->M_children[pred(cur->M_value)];
      if (!tmp) break;
      // fprintf(stderr, "tmp: %d\n", tmp->M_value);
      cur = tmp;
    }
    // fprintf(stderr, "-> cur: %p\n", cur.get());
    if (pred(cur->M_value)) cur = S_next(cur, this);
    // fprintf(stderr, "--> cur: %p\n", cur.get());
    // if (cur) {
    //   fprintf(stderr, "cur: %d (%p)\n", cur->M_value, cur.get());
    // }
    M_root = S_splay(cur? cur: M_rightmost);
    return iterator(cur, this);
  }
};

int arc033_c() {
  size_t q;
  scanf("%zu", &q);

  order_statistic_tree<int> ost;
  for (size_t i = 0; i < q; ++i) {
    int t;
    scanf("%d", &t);

    if (t == 1) {
      int x;
      scanf("%d", &x);
      ost.insert(ost.partition_point([x](int y) { return y < x; }), x);
    } else if (t == 2) {
      ptrdiff_t x;
      scanf("%td", &x);
      --x;
      auto it = ost.begin() + x;
      printf("%d\n", *it);
      ost.erase(it);
    }

    // fprintf(stderr, "size: %zu\n", ost.size());
    // for (auto it = ost.begin(); it != ost.end(); ++it)
    //   fprintf(stderr, "%d%c", *it, (std::next(it) != ost.end())? ' ': '\n');
  }
  return 0;
}

#include <vector>
#include <numeric>

int aoj2890() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);

  std::vector<int> a(n);
  for (auto& ai: a) scanf("%d", &ai), --ai;
  order_statistic_tree<int> ost(a.begin(), a.end());

  std::vector<typename order_statistic_tree<int>::const_iterator> its(n);
  {
    for (auto it = ost.begin(); it != ost.end(); ++it)
      its[*it] = it;
  }

  for (size_t i = 0; i < q; ++i) {
    size_t j;
    scanf("%zu", &j);
    --j;

    auto center = ost.split(its[j]);
    auto right = center.split(++center.begin());
    center.merge(ost);
    right.merge(center);
    ost = std::move(right);
  }

  for (auto it = ost.begin(); it != ost.end(); ++it)
    printf("%d%c", *it+1, (std::next(it)!=ost.end())? ' ': '\n');

  return 0;
}

int aoj_itp2_7_c() {
  size_t q;
  scanf("%zu", &q);

  order_statistic_tree<int> ost;
  for (size_t i = 0; i < q; ++i) {
    int t;
    scanf("%d", &t);

    if (t == 0) {
      // insert(x)
      int x;
      scanf("%d", &x);
      auto it = ost.partition_point([x](int y) { return x > y; });
      if (it == ost.end() || *it != x) ost.insert(it, x);
      // fprintf(stderr, "inserted %d\n", x);
      printf("%zu\n", ost.size());
    } else if (t == 1) {
      // find(x)
      int x;
      scanf("%d", &x);
      auto it = ost.partition_point([x](int y) { return x > y; });
      printf("%d\n", (it != ost.end() && *it == x));
    } else if (t == 2) {
      // delete(x)
      int x;
      scanf("%d", &x);
      auto it = ost.partition_point([x](int y) { return x > y; });
      if (it != ost.end() && *it == x) ost.erase(it);
      // fprintf(stderr, "erased %d\n", x);
    } else if (t == 3) {
      // dump(x, y)
      int x, y;
      scanf("%d %d", &x, &y);
      auto it = ost.partition_point([x](int z) { return x > z; });
      while (it != ost.end() && *it <= y)
        printf("%d\n", *it++);
    }
  }
  return 0;
}

int main() {
  arc033_c();
}
