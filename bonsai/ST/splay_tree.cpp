template <typename Tp>
class order_statistic_tree {
  // based on splay tree

public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;
  class itertor;
  class const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  class node;
  using pointer = std::shared_ptr<node>;

  class node {
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
      if (M_children[0]) delete M_children[0];
      if (M_children[1]) delete M_children[1];
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

    S_splay(ptr);
    ptr = ptr->M_children[1];
    --n;
    while (true) {
      difference_type ls = ptr->left_size();
      if (n == ls) return ptr;
      if (n < ls) {
        ptr = ptr->M_children[0];
      } else {
        ptr = ptr->M_children[1];
        n -= ls+1;
      }
    }
  }
  static pointer S_prev(pointer ptr, order_statistic_tree* tree,
                        difference_type n = 1) {

    if (n == 0) return ptr;
    if (n < 0) return S_next(ptr, tree, -n);
    if (!ptr)
      return S_prev(tree->M_rightmost, tree, n-1);  // past-the-end

    S_splay(ptr);
    ptr = ptr->M_children[0];
    --n;
    while (true) {
      difference_type rs = ptr->right_size();
      if (n == rs) return ptr;
      if (n < rs) {
        ptr = ptr->M_children[1];
      } else {
        ptr = ptr->M_children[0];
        n -= rs+1;
      }
    }
  }

  static size_type S_index(pointer ptr, order_statistic_tree* tree) {
    if (!ptr) return tree->size();
    S_splay(ptr);
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

    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub;
    if (sub) sub->M_parent = par;

    par->set_size();
    cur->set_size();
    return cur;
  }
  static pointer S_zigzig(pointer ptr, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[!dir];
    pointer sub2 = par->M_children[!dir];

    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub1;
    if (sub1) = sub1->M_parent = par;
    par->M_children[!dir] = gpar;
    gpar->M_parent = par;
    gpar->M_children[dir] = sub2;
    if (sub2) = sub2->M_parent = gpar;

    gpar->set_size();
    par->set_size();
    cur->set_size();
    return cur;
  }
  static pointer S_zigzag(pointer ptr, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[dir];
    pointer sub2 = cur->M_children[!dir];

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
  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp&;
    using const_reference = Tp const&;
    using pointer = std::shared_ptr<node>;

  private:
    pointer M_ptr = nullptr;
    mutable order_statistic_tree* M_tree = nullptr;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(pointer ptr, order_statistic_tree* tree):
      M_ptr(ptr), M_tree(tree) {}

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

    reference operator *() const {
      M_tree->M_root = S_splay(M_ptr); return M_value;
    }
  };

public:
  class const_iterator {
  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp const&;
    using const_reference = Tp const&;
    using pointer = std::shared_ptr<node>;

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
      M_tree->M_root = S_splay(M_ptr); return M_value;
    }
  };

private:
  size_type M_size = 0;
  pointer M_root = nullptr;
  pointer M_leftmost = nullptr;
  pointer M_rightmost = nullptr;

  void M_init_ends() {
    M_leftmost = M_rightmost = M_root;
    while (M_leftmost->M_children[0])
      M_leftmost = M_leftmost->M_children[0];
    while (M_rightmost->M_children[1])
      M_rightmost = M_rightmost->M_children[1];
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

public:
  order_statistic_tree() = default;
  order_statistic_tree(order_statistic_tree const& other) {
    M_deep_copy(other);
  }
  order_statistic_tree(order_statistic_tree&&) = default;

  template <typename InputIt>
  order_statistic_tree(InputIt first, InputIt last) { assign(first, last); }
  order_statistic_tree(std::initializer_list<value_type> il) { assign(il); }

  order_statistic_tree(size_type size, const_reference value) {
    assign(size, value);
  }

  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    clear();
    while (first != last) {
      pointer tmp = M_root;
      M_root = std::make_shared<node>(*first++);
      M_root->M_children[0] = tmp;
      M_root->M_size = ++M_size;
      tmp->M_parent = M_root;
    }
    M_init_ends();
  }
  void assign(std::initializer_list<value_type> il) {
    assign(il.begin(), il.end());
  }

  void assign(size_type size, const_reference value) {
    clear();
    for (size_type i = 0; i < size; ++i) {
      pointer tmp = M_rightmost;
      M_root = std::make_shared<node>(value);
      M_root->M_children[0] = tmp;
      M_root->M_size = ++M_size;
      tmp->M_parent = M_root;
    }
    M_init_ends();
  }

  void clear() {
    M_size = 0;
    M_root = M_rightmost = nullptr;
  }

  order_statistic_tree& operator =(order_statistic_tree const& other) {
    M_deep_copy(other);
    return *this;
  }
  order_statistic_tree& operator =(order_statistic_tree&&) = default;

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
  const_reverse_iterator rend() const { return cbegin(); }

  bool empty() const noexcept { return M_size == 0; }
  size_type size() const { return M_size; }

  iterator insert(const_iterator pos, const_reference value) {
    S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(value);
    merge(std::move(tmp));
    return res;
  }

  iterator insert(const_iterator pos, value_type&& value) {
    S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(std::move(value));
    merge(std::move(tmp));
    return res;
  }

  iterator insert(const_iterator pos, size_type count, const_reference value) {
    if (count == 0) return pos;
    S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(value);
    for (size_type i = 1; i < count; ++i) push_back(value);
    merge(std::move(tmp));
    return res;
  }

  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    if (first == last) return pos;
    S_splay(pos.M_ptr);
    order_statistic_tree tmp = split(pos);
    iterator res = push_back(*first++);
    while (first != last) push_back(*first++);
    merge(std::move(tmp));
    return res;
  }

  iterator insert(const_iterator pos, std::initializer_list<value_type> il) {
    return insert(pos, il.begin(), il.end());
  }
};
