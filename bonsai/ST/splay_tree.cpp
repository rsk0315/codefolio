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
  };

  static pointer S_next(pointer ptr, difference_type n = 1);
  static pointer S_prev(pointer ptr, difference_type n = 1);

  static size_type S_index(pointer ptr);

  static size_type S_parent_dir(pointer ptr) {
    if (!ptr->M_parent) return -1_zu;
    if (ptr->M_parent->M_children[0] == ptr) return 0;
    if (ptr->M_parent->M_children[1] == ptr) return 1;
    return -1_zu;
  }

  static pointer S_zig(pointer ptr, size_type dir) {
    
  }
  static pointer S_zigzig(pointer ptr, size_type dir);
  static pointer S_zigzag(pointer ptr, size_type dir);

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
