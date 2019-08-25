#include <cstddef>
#include <array>
#include <memory>

template <typename Tp>
class balanced_tree {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;
  class const_iterator;
  class iterator;

private:
  enum S_color {S_black, S_red};
  struct node;
  using base_ptr = std::shared_ptr<node>;
  using const_base_ptr = std::shared_ptr<node const>;

public:
  class const_iterator {
  public:
    friend iterator;
    using size_type = typename balanced_tree::size_type;
    using difference_type = typename balanced_tree::difference_type;
    using value_type = typename balanced_tree::value_type;
    using reference = typename balanced_tree::const_reference;

  private:
    using node = typename balanced_tree::node;
    using base_ptr = typename balanced_tree::const_base_ptr;
    using mutable_base_ptr = typename balanced_tree::base_ptr;
    base_ptr M_ptr;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    const_iterator(const_base_ptr ptr): M_ptr(ptr) {}
    const_iterator(mutable_base_ptr ptr):
      M_ptr(std::const_pointer_cast<node const>(ptr)) {}
    const_iterator(iterator const& other):
      M_ptr(std::const_pointer_cast<node const>(other.M_ptr)) {}

    const_iterator& operator +=(difference_type n) {
      S_advance(M_ptr, n); return *this;
    }
    const_iterator& operator ++() { return *this += 1; }
    const_iterator operator +(difference_type n) const {
      return const_iterator(*this) += n;
    }
    const_iterator& operator -=(difference_type n) {
      S_advance(M_ptr, n); return *this;
    }
    const_iterator& operator --() { return *this -= 1; }
    const_iterator operator -(difference_type n) const {
      return const_iterator(*this) -= n;
    }
    difference_type operator -(const_iterator const& other) const {
      return S_index(M_ptr) - S_index(other.M_ptr);
    }
  };

  class iterator {
  public:
    friend const_iterator;
    using size_type = typename balanced_tree::size_type;
    using difference_type = typename balanced_tree::difference_type;
    using value_type = typename balanced_tree::value_type;
    using reference = typename balanced_tree::reference;

  private:
    using node = typename balanced_tree::node;
    using base_ptr = typename balanced_tree::base_ptr;
    using const_ptr = typename balanced_tree::const_base_ptr;
    base_ptr M_ptr;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(base_ptr ptr): M_ptr(ptr) {}
    iterator(const_base_ptr ptr):
      M_ptr(std::const_pointer_cast<node>(ptr)) {}

    iterator& operator +=(difference_type n) {
      S_advance(M_ptr, n); return *this;
    }
    iterator& operator ++() { return *this += 1; }
    iterator operator +(difference_type n) const {
      return iterator(*this) += n;
    }
    iterator& operator -=(difference_type n) {
      S_advance(M_ptr, n); return *this;
    }
    iterator& operator --() { return *this -= 1; }
    iterator operator -(difference_type n) const {
      return iterator(*this) -= n;
    }
    difference_type operator -(const_iterator const& other) const {
      return S_index(M_ptr) - S_index(other.M_ptr);
    }
  };

private:
  struct node {
    using base_ptr = std::shared_ptr<node>;

    value_type value;
    S_color color = S_black;
    size_type rank = 0;
    size_type size = 1;
    std::array<base_ptr, 2> children{{nullptr, nullptr}};

    node() = default;
    node(const_reference value): value(value) {}
    node(base_ptr left, base_ptr right, S_color color):
      color(color), children{{left, right}} { maintain(); }

    void maintain() {
      base_ptr left = children[0];
      base_ptr right = children[1];
      if (!left) return;
      size = left->size + right->size;
      rank = std::max(left->rank + ((left->color == S_black)? 1: 0),
                      right->rank + ((right->color == S_black)? 1: 0));
    }
  };

  static difference_type S_index(base_ptr v) {
    // これを求めるために node は parent を管理してなきゃいけない気がする
  }

  static void S_advance(base_ptr& v, difference_type n) {
    // これも parent がほしい
  }

  static base_ptr S_rotate(base_ptr v, size_t d) {
    base_ptr w = v->children[d];
    v->children[d] = w->children[d^1];
    w->children[d^1] = v;
    v->children[d]->maintain();
    v->maintain();
    w->maintain();
    v->color = S_red;
    w->color = S_black;
    return w;
  }

  static base_ptr S_merge_dfs(base_ptr u, base_ptr v) {
    if (u->rank < v->rank) {
      base_ptr w = S_merge_dfs(u, v->children[0]);
      v->children[0] = w;
      v->maintain();
      if (v->color == S_black && w->color == S_red && w->children[0]->color == S_red) {
        if (v->children[1]->color == S_black) return S_rotate(v, 0);
        v->color = S_red;
        v->children[0]->color = v->chidlren[1]->color = S_black;
        return v;
      }
      return v;
    }
    if (u->rank > v->rank) {
      base_ptr w = S_merge_dfs(u->children[1], v);
      u->children[1] = w;
      u->maintain();
      if (u->color == S_black && w->color == S_red && w->children[1]->color == S_red) {
        if (u->children[0]->color == S_black) return S_rotate(u, 1);
        u->color = S_red;
        u->children[0]->color = u->children[1]->color = S_black;
        return u;
      }
      return u;
    }
    return make_shared(u, v, S_red);
  }

  static base_ptr S_merge(base_ptr u, base_ptr v) {
    if (!u) return v;
    if (!v) return u;
    u = S_merge_dfs(u, v);
    u->color = S_black;
    return u;
  }

  static std::pair<base_ptr, base_ptr> S_split(base_ptr v, size_type k) {
    if (k == 0) return {nullptr, v};
    if (k == size()) return {v, nullptr};
    if (k < v->children[0]->size) {
      auto p = S_split(v->children[0], k);
      return {p.first, S_merge(p.second, v->children[1])};
    }
    if (k > v->children[0]->size) {
      auto p = S_split(v->children[1], k - v->left->size);
      return {S_merge(v->children[0], p.first), p.second};
    }
    return {v->children[0], v->children[1]};
  }

public:
  balanced_tree() { clear(); }
  balanced_tree(balanced_tree const&) = default;
  balanced_tree(balanced_tree&&) = default;
  template <typename InputIt>
  balanced_tree(InputIt first, InputIt last) {
    assign(first, last);
  }
  balanced_tree(size_type count, const_reference value) {
    assign(count, value);
  }
  balanced_tree(std::initializer_list<value_type> ilist) {
    assign(ilist);
  }

  balanced_tree& operator =(balanced_tree const& other);  // deep copy
  balanced_tree& operator =(balanced_tree&&) = default;

  void assign(size_type count, const_reference value) {
    clear();
    while (count--) push_back(value);
  }
  template <typename InputIt>
  void assign(InputIt first, InputIt last) {
    clear();
    while (first != last) push_back(*first++);
  }
  void assign(std::initializer_list<value_type> ilist) {
    clear();
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
  template <typename Predicate>
  iterator partition_point(const_reference key, Predicate pred);
  template <typename Predicate>
  const_iterator partition_point(const_reference key, Predicate pred) const;
};
