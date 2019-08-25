template <typename Tp>
class balanced_tree {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;
  class iterator;
  class const_iterator;

private:

public:
  balanced_tree() { M_put_sentinel(); }
  balanced_tree(balanced_tree const&) = default;
  balanced_tree(balanced_tree&&) = default;
  template <typename InputIt>
  balanced_tree(InputIt first, InputIt last): balanced_tree() {
    while (first != last) push_back(*first++);
  }
  balanced_tree(size_type count, const_reference value): balanced_tree() {
    while (count--) push_back(value);
  }
  balanced_tree(std::initializer_list<value_type> ilist): balanced_tree() {
    for (auto const& x: ilist) push_back(x);
  }

  balanced_tree& operator =(balanced_tree const& other);  // deep copy
  balanced_tree& operator =(balanced_tree&&) = default;

  // assign
  void assign(size_type count, const_reference value) {
    clear();
    M_put_sentinel();
    while (count--) push_back(value);
  }
  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    clear();
    M_put_sentinel();
    while (first != last) push_back(*first++);
  }
  void assign(std::initializer_list<value> ilist) {
    clear();
    M_put_sentinel();
    for (auto x: ilist) push_back(x);
  }

  reference at(size_type pos);
  const_reference at(size_type pos) const;
  reference operator [](size_type pos);
  const_reference operator [](size_type pos) const;

  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;

  bool empty() const;
  size_type size() const noexcept;

  void clear();

  iterator insert(const_iterator pos, const_reference x);
  iterator insert(const_iterator pos, value_type&& x);
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last);
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist);

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args);

  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);

  void push_front(const_reference value);
  void push_front(value_type&& value);
  template <typename... Args>
  reference emplace_front(Args&&... args);

  void push_back(const_reference value);
  void push_back(value_type&& value);
  template <typename... Args>
  reference emplace_back(Args&&... args);

  void pop_front();
  void pop_back();

  void resize(size_type count);
  void resize(size_type count, const_reference value);

  void splice(const_iterator pos, balanced_tree& other);
  void splice(const_iterator pos, balanced_tree&& other);
  void splice(const_iterator pos, balanced_tree& other, const_iterator it);
  void splice(const_iterator pos, balanced_tree&& other, const_iterator it);
  void splice(const_iterator pos, balanced_tree& other,
              const_iterator first, const_iterator last);
  void splice(const_iterator pos, balanced_tree&& other,
              const_iterator first, const_iterator last);

  void merge(balanced_tree& source);
  void merge(balanced_tree&& source);
  balanced_tree split(const_iterator pos);

  iterator lower_bound(const_reference key);
  const_iterator lower_bound(const_reference key) const;
  template <typename Compare>
  iterator lower_bound(const_reference key, Compare comp);
  template <typename Compare>
  const_iterator lower_bound(const_reference key, Compare comp) const;

  iterator upper_bound(const_reference key);
  const_iterator upper_bound(const_reference key) const;
  template <typename Compare>
  iterator upper_bound(const_reference key, Compare comp);
  template <typename Compare>
  const_iterator upper_bound(const_reference key, Compare comp) const;

  iterator partition_point(const_reference key);
  const_iterator partition_point(const_reference key) const;
  template <typename Compare>
  iterator partition_point(const_reference key, Compare comp);
  template <typename Compare>
  const_iterator partition_point(const_reference key, Compare comp) const;
};
