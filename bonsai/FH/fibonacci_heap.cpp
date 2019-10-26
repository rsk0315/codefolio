template <typename Tp, typename Compare = std::less<>>
class fibonacci_heap {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;
  using compare = Compare;

private:

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

  const_reference top() const;
  void pop();
  node_handle push(const_reference key);
  node_handle push(value_type&& key);

  template <typename... Args>
  node_handle emplace(Args...&& args);

  // node_handle&? node_handle const&?
  node_handle update(node_handle& nh, const_reference key);
};
