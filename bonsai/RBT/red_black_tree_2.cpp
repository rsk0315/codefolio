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
  using base_ptr = Tp*;
  class node_type;
  using node_ptr = node_type*;

  void M_deep_copy(rb_tree& dst, rb_tree const& src);
  void M_release();
  void M_clear();

  base_ptr M_insert(const_iterator pos, node_ptr newnode);
  void M_insert_fix(const_iterator pos);

  base_ptr M_erase(const_iterator pos);
  void M_erase_fix(const_iterator pos, const_iterator parent);

  // M_merge();
  // M_split();

  void M_calculate_size(const_iterator subroot) const;
  void M_fix_left_subtree_size(const_iterator descendant, difference_type diff);

public:
  rb_tree() = default;
  rb_tree(rb_tree&&) = default;
  rb_tree(rb_tree const& other) { M_deep_copy(*this, other); }
  rb_tree(size_type n, value_type const& x = value_type{}) {
    for (size_t i = 0; i < n; ++i) push_back(x);
  }
  template <typename InputIt>
  rb_tree(InputIt first, InputIt last) {
    while (first != last) push_back(*first++);
  }

  rb_tree& operator =(rb_tree&&) = default;
  rb_tree& operator =(rb_tree const& other) { M_deep_copy(*this, other); }

  // insertions and an emplacement
  iterator insert(const_iterator pos, value_type const& value);
  iterator insert(const_iterator pos, value_type&& value);
  iterator insert(const_iterator pos, size_type count, value_type const& value);
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last);
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist);
  template <typename... Args>
  iterator emplace(Args&&... args);
  iterator push_back(value_type const& value);
  iterator push_back(value_type&& value);
  iterator push_front(value_type const& value);
  iterator push_front(value_type&& value);

  // erasures
  iterator erase(iterator pos);
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
  iterator pop_front();
  iterator pop_back();

  // merge and split
  iterator merge(rb_tree&& other);
  rb_tree split(iterator pos);

  // other modifier(s)
  void clear();

  // boundaries
  template <typename Comparator>
  const_iterator lower_bound(value_type const& key, Comparator comp) const;
  template <typename Comparator>
  const_iterator upper_bound(value_type const& key, Comparator comp) const;

  // basic iterators
  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;

  // capacities
  bool empty() const;
  size_type size() const;

  // accessors
  size_type index(const_iterator pos) const;
  reference operator [](size_type pos);
  const_reference operator [](size_type pos) const;
  reference at(size_type pos);
  const_reference at(size_type pos) const;
  // the i-th iterator should be obtained via begin()+i
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;
};
