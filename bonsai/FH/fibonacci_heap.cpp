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

public:
  class node_handle {
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
    const_reference operator *() const { return M_node->M_value; }
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
    for (auto r: M_roots) size += 1_zu << r.M_order;
    std::vector<pointer> roots(bit::log2p1(size));
    for (auto r: M_roots) {
      size_type i = r.M_order;
      while (roots[i]) {
        r->add_child(roots[i]);
        roots[i] = nullptr;
        ++i;
      }
      roots[i] = r;
    }
    M_roots.clear();
    for (auto r: roots) if (r) M_roots.push_back(r);
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
  fibonacci_heap(std::initializer_list<value_type> il) { assign(ilist); }

  size_type size() const noexcept { return M_size; }
  [[nodiscard]] bool empty() const noexcept { return M_size == 0; }

  const_reference const& top() const { return M_top->M_value; }
  void pop() {
    pointer root = *M_top;
    M_roots.erase(M_top);
    if (root->M_child) {
      pointer cur = root->M_child;
      do {
        pointer tmp = cur->right;
        M_roots.push_back(cur);
        cur->left = cur->right = nullptr;
        cur = tmp;
      } while (cur != root->M_child);
    }
    M_coleasce();
  }

  node_handle push(key_type const& key, mapped_type const& mapped) {
    ++M_size;
    pointer newnode = std::make_shared<node>(key, mapped);
    M_roots.push_back(newnode);
    if (M_comp(M_top->M_value.first, key)) M_top = newnode;
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
