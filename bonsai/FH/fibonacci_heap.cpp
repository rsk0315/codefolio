template <typename Key, typename Tp, typename Compare = std::less<>>
class fibonacci_heap {
public:
  using size_type = size_t;
  using key_type = Key;
  using mapped_type = Tp;
  using value_type = std::pair<key_type, mapped_type>;
  using reference = value_type&;
  using const_reference = value_type const&;
  using key_compare = Compare;
  class node_handle;

private:
  class node;
  using pointer = std::shared_ptr<node>;

  class node {
    friend fibonacci_heap;

  public:
    using size_type = size_t;
    using key_type = Key;
    using mapped_type = Tp;
    using value_type = std::pair<key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = value_type const&;
    using key_compare = Compare;

  private:
    pointer M_parent{nullptr};
    pointer M_child{nullptr};
    pointer M_left{nullptr};
    pointer M_right{nullptr};
    bool M_damaged = false;
    size_type M_order = 0;
    value_type M_value;  // {priority, element}

  public:
    node() = default;
    node(node const&) = default;
    node(node&&) = default;
    node& operator =(node const&) = default;
    node& operator =(node&&) = default;

    node(key_type const& key, mapped_type const& mapped):
      M_value(key, mapped) {}
  };

  static void S_add_child(pointer parent, pointer child) {
    ++parent->M_order;
    child->M_parent = parent;
    if (!parent->M_child) {
      parent->M_child = child;
      child->M_left = child->M_right = nullptr;
    } else if (!parent->M_child->M_left) {
      parent->M_child->M_left = parent->M_child->M_right = child;
      child->M_left = child->M_right = parent->M_child;
    } else {
      child->M_left = parent->M_child->M_left;
      child->M_right = parent->M_child;
      parent->M_child->M_left->M_right = child;
      parent->M_child->M_right->M_left = child;
    }
  }

public:
  class node_handle {
    friend fibonacci_heap;

  public:
    using size_type = size_t;
    using key_type = Key;
    using mapped_type = Tp;
    using value_type = std::pair<key_type, mapped_type>;
    using reference = value_type&;
    using const_reference = value_type const&;
    using key_compare = Compare;
    using pointer = std::weak_ptr<node>;

  private:
    pointer M_node;

  public:
    node_handle(pointer node): M_node(node) {}
    const_reference operator *() const { return (*M_node)->M_value; }
  };

private:
  size_type M_size = 0;
  std::list<pointer> M_roots;  // list for pop()
  typename std::list<pointer>::iterator M_top;
  key_compare M_comp = key_compare();

  static void S_deep_copy_dfs(pointer& dst, pointer& src) {
    // ???
  }

  void M_deep_copy(fibonacci_heap const& other) {
    M_size = other.M_size;
    M_roots.clear();
    for (auto const& root: other.M_roots) {
      pointer r;
      S_deep_copy_dfs(root);
      M_roots.push_back(r);
      if (root == *other.M_top) M_top = --M_roots.end();
    }
  }

  void M_coleasce() {
    // ??? use M_damage?
    size_type size = 0;
    for (auto r: M_roots) size += 1_zu << r->M_order;
    std::vector<pointer> roots(bit::log2p1(size));

    for (auto r: M_roots) {
      size_type i = r->M_order;
      while (roots[i]) {
        S_add_child(r, roots[i]);
        roots[i] = nullptr;
        ++i;
      }
      roots[i] = r;
    }
    M_roots.clear();

    if (M_size == 0) return;
    for (auto r: roots) if (r) M_roots.push_back(r);
    M_top = M_roots.begin();
    for (auto it = ++M_roots.begin(); it != M_roots.end(); ++it)
      if (M_comp((*M_top)->M_value.first, (*it)->M_value.first))
        M_top = it;
  }

public:
  fibonacci_heap() = default;
  fibonacci_heap(fibonacci_heap const& other) { M_deep_copy(other); }
  fibonacci_heap(fibonacci_heap&&) = default;
  fibonacci_heap& operator =(fibonacci_heap const& other) {
    M_deep_copy(other);
  }
  fibonacci_heap& operator =(fibonacci_heap&&) = default;

  template <typename InputIt>
  fibonacci_heap(InputIt first, InputIt last) { assign(first, last); }
  fibonacci_heap(std::initializer_list<value_type> il) { assign(il); }

  size_type size() const noexcept { return M_size; }
  [[nodiscard]] bool empty() const noexcept { return M_size == 0; }

  const_reference const& top() const { return (*M_top)->M_value; }
  void pop() {
    fprintf(stderr, "before pop(): M_top->use_count(): %ld\n", M_top->use_count());
    pointer root = *M_top;
    fprintf(stderr, "before pop(): root.use_count(): %ld\n", root.use_count());
    M_roots.erase(M_top);
    if (root->M_child) {
      pointer cur = root->M_child;
      cur->M_parent = nullptr;
      do {
        pointer tmp = cur->M_right;
        M_roots.push_back(cur);
        cur->M_left = cur->M_right = cur->M_parent = nullptr;
        cur = tmp;
      } while (cur != root->M_child);
    }
    --M_size;
    fprintf(stderr, "before M_coleasce(): root.use_count(): %ld\n", root.use_count());
    M_coleasce();
    fprintf(stderr, "after M_coleasce(): root.use_count(): %ld\n", root.use_count());
  }

  node_handle push(key_type const& key, mapped_type const& mapped) {
    pointer newnode = std::make_shared<node>(key, mapped);
    M_roots.push_back(newnode);
    if (M_size == 0 || M_comp((*M_top)->M_value.first, key))
      M_top = --M_roots.end();
    ++M_size;
    return node_handle(newnode);
  }

  void meld(fibonacci_heap& other) {
    M_size += other.M_size;
    other.M_size = 0;

    M_roots.splice(M_roots.end(), other.M_roots);
    if (M_comp(M_top->M_value.first, other.M_top->M_value.first))
      M_top = other.M_top;
  }

  // node_handle&? node_handle const&?
  node_handle prioritize(node_handle& nh, mapped_type const& key);
};
