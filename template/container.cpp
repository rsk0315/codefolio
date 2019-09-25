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
};
