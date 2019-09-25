template <typename Tp>
class container {
public:
  using value_type = Tp;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = Tp&;
  using const_reference = Tp const&;
  class iterator;
  class const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  class iterator {
  public:
    using value_type = Tp;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = Tp&;
    // using pointer;

    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    // iterator(pointer);

    iterator& operator ++();
    iterator operator ++(int) {
      iterator tmp(*this); ++*this; return tmp;
    }
    iterator& operator --();
    iterator operator --(int) {
      iterator tmp(*this); ++*this; return tmp;
    }

    bool operator ==(const_iterator const& rhs) const;
    bool operator !=(const_iterator const& rhs) const {
      return !(*this == rhs);
    }

    // for random-access iterators
    iterator& operator +=(difference_type);
    iterator operator +(difference_type n) const {
      return iterator(*this) += n;
    }
    iterator& operator -=(difference_type);
    iterator operator -(difference_type n) const {
      return iterator(*this) -= n;
    }
    difference_type operator -(const_iterator const &);

    bool operator <(const_iterator const& rhs) const;
    bool operator <=(const_iterator const& rhs) const {
      return !(rhs < *this);
    }
    bool operator >(const_iterator const& rhs) const {
      return rhs < *this;
    }
    bool operator >=(const_iterator const& rhs) const {
      return !(*this < rhs);
    }
  };
  class const_iterator {
    using value_type =  Tp;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = Tp const&;
    // using pointer;  // const

    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    // const_iterator(pointer);
    const_iterator(iterator const&);
    const_iterator(iterator&&);

    const_iterator& operator ++();
    const_iterator operator ++(int) {
      const_iterator tmp(*this); ++*this; return tmp;
    }
    const_iterator& operator --();
    const_iterator operator --(int) {
      const_iterator tmp(*this); --*this; return tmp;
    }

    bool operator ==(const_iterator const& rhs) const;
    bool operator !=(const_iterator const& rhs) const {
      return !(*this == rhs);
    }

    // for random-access iterators
    const_iterator& operator +=(difference_type);
    const_iterator operator +(difference_type n) const {
      return const_iterator(*this) += n;
    }
    const_iterator& operator -=(difference_type);
    const_iterator operator -(difference_type n) const {
      return const_iterator(*this) -= n;
    }
    difference_type operator -(const_iterator const &);

    bool operator <(const_iterator const& rhs) const;
    bool operator <=(const_iterator const& rhs) const {
      return !(rhs < *this);
    }
    bool operator >(const_iterator const& rhs) const {
      return rhs < *this;
    }
    bool operator >=(const_iterator const& rhs) const {
      return !(*this < rhs);
    }
  };

private:
  size_type M_size = 0;

public:
  // constructor(s) and destructor
  container();
  container(container const&);
  container(container&&) = default;

  // iterators
  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;

  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  // capacity
  [[nodiscard]] bool empty() const noexcept { return M_size == 0; }
  size_type size() const noexcept { return M_size; }
  void resize(size_type size);
  void resize(size_type size, const_reference value);

  // element access
  reference operator [](size_type n);
  const_reference operator [](size_type n) const;
  reference at(size_type n);
  const_reference at(size_type n) const;
  reference front();
  const_reference front() const;
  
  // copying, moving and assignment functions
  container& operator =(container const&);
  container& operator =(container&&) = default;
  template <typename InputIt>
  void assign(InputIt first, InputIt last);
  void assign(std::initializer_list<value_type> il) {
    assign(il.begin(), il.end());
  }
  void assign(size_type count, const_reference value);

  // modifier functions
  template <class... Args>
  reference emplace_front(Args&&... args);
  template <class... Args>
  reference emplace_back(Args&&... args);
  void push_front(const_reference value);
  void push_front(value_type&& value);
  void push_back(const_reference value);
  void push_back(value_type&& value);
  void pop_front();
  void pop_back();

  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args);
  iterator insert(const_iterator pos, const_reference value);
  iterator insert(const_iterator pos, value_type&& value);
  iterator insert(const_iterator pos, size_type count, const_reference value);
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last);
  iterator insert(const_iterator pos, std::initializer_list<value_type> il);
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);

  void swap(container&);
  void clear() noexcept;

  // observer functions

  // specific operations
  template <typename Predicate>
  iterator partition_point(Predicate pred);

  void splice(const_iterator pos, container& source);
  void splice(const_iterator pos, container&& source);
  void splice(const_iterator pos, container& source, const_iterator it);
  void splice(const_iterator pos, container&& source, const_iterator it);
  void splice(const_iterator pos, container& source,
              const_iterator first, const_iterator last);
  void splice(const_iterator pos, container&& source,
              const_iterator first, const_iterator last);

  void merge(container& source);
  void merge(container&& source);
  container split(const_iterator pos);
  
  // operators
  bool operator ==(container const& other) const;
  bool operator !=(container const& other) const {
    return !(*this == other);
  }
  bool operator <(container const& other) const;
  bool operator <=(container const& other) const {
    return !(other < *this);
  }
  bool operator >(container const& other) const {
    return other < *this;
  }
  bool operator >=(container const& other) const {
    return !(*this < other);
  }
};
