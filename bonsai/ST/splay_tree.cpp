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
    node(node const&) = default;
    node(node&&) = default;
    node(const_reference value): M_value(value) {}
    node(value_type&& value): M_value(std::move(value)) {}

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
    iterator(pointer ptr): M_ptr(ptr) {}

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
    const_iterator(pointer ptr): M_ptr(ptr) {}

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

    difference_type operator -(const_const_iterator const& other) const {
      return S_index(M_ptr, M_tree) - S_index(other.M_ptr, M_tree);
    }

    bool operator ==(const_const_iterator const& other) const {
      return M_ptr == other.M_ptr;
    }
    bool operator !=(const_const_iterator const& other) const {
      return !(*this == other);
    }

    bool operator <(const_const_iterator const& other) const {
      return S_index(M_ptr, M_tree) < S_index(other.M_ptr, M_tree);
    }
    bool operator >(const_const_iterator const& other) const {
      return other < *this;
    }
    bool operator <=(const_const_iterator const& other) const {
      return !(other < *this);
    }
    bool operator >=(const_const_iterator const& other) const {
      return !(*this < other);
    }

    reference operator *() const {
      M_tree->M_root = S_splay(M_ptr); return M_value;
    }
  };

private:

public:
};
