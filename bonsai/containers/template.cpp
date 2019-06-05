#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <utility>
#include <memory>
#include <array>
#include <iterator>

template <typename Tp>
class container {
public:
  using value_type = Tp;
  using reference = value_type&;
  using const_reference = value_type const&;
  using difference_type = ptrdiff_t;
  using size_type = size_t;
  class iterator;
  class const_iterator;

private:
  struct M_node {
    value_type value{};
    // like this
    // size_type height = 0;
    // using link_type = std::shared_ptr<M_node>;
    // link_type parent = parent;
    // std::array<link_type, 2> children = {nullptr, nullptr};
  };

  // using link_type = typename M_node::link_type;
  // using base_ptr = std::shared_ptr<M_node>;
  // using const_base_ptr = std::shared_ptr<M_node const>;

  base_ptr M_begin = nullptr;
  base_ptr M_end = nullptr;

public:
  class iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;  // or something else
    using value_type = typename container::value_type;
    using difference_type = typename container::difference_type;
    using pointer = std::shared_ptr<typename container::value_type>;
    using reference = typename container::reference;

  private:
    using base_ptr = typename container::base_ptr;
    base_ptr node = nullptr;

  public:
    iterator() = default;
    iterator(iterator const&) = default;
    iterator(iterator&&) = default;
    iterator(const_iterator const& other): node(other.node) {}
    iterator(const_iterator&& other): node(other.node) {}
    iterator& operator =(iterator const&) = default;
    iterator& operator =(iterator&&) = default;
  };

  class const_iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename container::value_type;
    using difference_type = typename container::difference_type;
    using pointer = std::shared_ptr<typename container::value_type const>;
    using reference = typename container::const_reference;

  private:
    using base_ptr = typename container::const_base_ptr;
    base_ptr node = nullptr;

  public:
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator&&) = default;
    const_iterator& operator =(const_iterator const&) = default;
    const_iterator& operator =(const_iterator&&) = default;
  };

  bool operator <(const_iterator const& lhs, const_iterator const& rhs) const;
  bool operator ==(const_iterator const& lhs, const_iterator const& rhs) const;

  bool operator <=(const_iterator const& lhs, const_iterator const& rhs) const {
    return !(rhs < lhs);
  }
  bool operator >(const_iterator const& lhs, const_iterator const& rhs) const {
    return (rhs < lhs);
  }
  bool operator >=(const_iterator const& lhs, const_iterator const& rhs) const {
    return !(lhs < rhs);
  }
  bool operator !=(const_iterator const& lhs, const_iterator const& rhs) const {
    return !(lhs == rhs);
  }

  // const_iterator += difference_type
  // const_iterator -= difference_type
  // const_iterator + difference_type
  // const_iterator - difference_type
  // ++const_iterator
  // --const_iterator
  // *const_iterator

private:
  const_base_ptr M_insert(const_base_ptr pos, const_base_ptr newnode);
  const_base_ptr M_erase(const_base_ptr pos);

  // static return_type S_function();
  // return_type M_function();
  // variable_type M_variable;
  // static variable_type S_variable;
  // enum S_enum { S_const1, S_const2, };

public:
  avl_tree() {
    M_prepare_sentinel();
  }
  avl_tree(avl_tree const&) = default;
  avl_tree(avl_tree&&) = default;
  template <typename InputIt>
  avl_tree(InputIt first, InputIt last) {
    M_prepare_sentinel();
    while (first != last) push_back(*first++);
  }
  avl_tree(std::initializer_list<value_type> ilist): avl_tree(ilist.begin(), ilist.end()) {}

  avl_tree& operator =(avl_tree const&) = default;
  avl_tree& operator =(avl_tree&&) = default;

  // # element access
  // ## at
  reference at(size_t i) {
    if (i >= size()) throw std::out_of_range("i >= size()");
    return *(begin() + i);
  }
  const_reference at(size_t i) const {
    if (i >= size()) throw std::out_of_range("i >= size()");
    return (*cbegin() + i);
  }
  // ## operator []
  reference operator [](size_t i) { return *(begin() + i); }
  const_reference operator [](size_t i) const { return *(cbegin() + i); }
  // ## front and back
  reference first() { return *begin(); }
  const_reference first() const { return *cbegin(); }
  reference back() { return *rbegin(); }
  const_reference back() const { return *crbegin(); }

  // # iterators
  // ## begin
  iterator begin() { return M_begin; }
  const_iterator begin() const { return M_begin; }
  const_iterator cbegin() const { return M_begin; }
  // ## end
  iterator end() { return M_end; }
  const_iterator end() const { return M_end; }
  const_iterator cend() const { return M_end; }
  // ## rbegin
  reverse_iterator rbegin() { return M_end; }
  const_reverse_iterator rbegin() const { return M_end; }
  const_reverse_iterator crbegin() const { return M_end; }
  // ## rend
  reverse_iterator rend() { return M_begin; }
  const_reverse_iterator rend() const { return M_begin; }
  const_reverse_iterator crend() const { return M_begin; }

  // # capacity
  // ## empty
  bool empty() const { return M_size == 0; }
  // ## size
  size_type size() const { return M_size; }

  // # modifiers
  // ## clear
  void clear() { M_clear(); }
  // ## insert and emplace
  iterator insert(const_iterator pos, value_type const& value) {
    return M_insert(pos.node, const_base_ptr(new M_node(value)));
  }
  iterator insert(const_iterator pos, value_type&& value) {
    return M_insert(pos.node, const_base_ptr(new M_node(std::move(value))));
  }
  iterator insert(const_iterator pos, size_t count, value_type const& value) {
    iterator res = pos;
    while (count--) res = M_insert(pos.node, const_base_ptr(new M_node(value)));
    return res;
  }
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    iterator res = pos;
    while (first != last) res = M_insert(pos.node, const_base_ptr(new M_node(*first++)));
    return res;
  }
  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
    return insert(pos, ilist.begin(), ilist.end());
  }
  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return M_insert(pos.node, const_base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  // ## erase
  iterator erase(iterator pos) { return M_erase(pos.node); }
  iterator erase(const_iterator pos) { return M_erase(pos.node); }
  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) first = erase(first);
    return first;
  }
  // ## push_back and push_front
  void push_back(value_type const& value) { insert(cend(), value); }
  void push_back(value_type&& value) { insert(cend(), std::move(value)); }
  void push_front(value_type const& value) { insert(cbegin(), value); }
  void push_front(value_type&& value) const { insert(cbegin(), std::move(value)); }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    M_insert(M_end, const_base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  template <typename... Args>
  void emplace_front(Args&&... args) {
    M_insert(M_begin, const_base_ptr(new M_node(std::forward<Args>(args)...)));
  }
  // ## pop_back and pop_front
  void pop_back() { erase(--end()); }
  void pop_front() { erase(begin()); }
  // ## merge and split
  iterator merge(container&& other);
  container split(const_iterator pos);
};
