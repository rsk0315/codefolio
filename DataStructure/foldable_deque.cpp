template <typename Monoid>
class foldable_deque {
public:
  using value_type = Monoid;
  using size_type = size_t;

private:
  using stack_type = std::stack<value_type, std::vector<value_type>>;
  stack_type M_front, M_back, M_front_folded, M_back_folded;

  void M_balance() {
    // precondition: (M_front.empty() || M_back.empty())
    std::deque<value_type> whole;
    while (!M_front.empty()) {
      whole.push_back(M_front.top());
      M_front.pop();
      M_front_folded.pop();
    }
    while (!M_back.empty()) {
      whole.push_front(M_back.top());
      M_back.pop();
      M_back_folded.pop();
    }

    size_type half = whole.size() / 2;
    for (size_type i = half; i--;) {
      value_type x = std::move(whole[i]);
      M_front.push(x);
      if (!M_front_folded.empty()) x.fold_eq(M_front_folded.top());
      M_front_folded.push(x);
    }
    for (size_type i = half; i < whole.size(); ++i) {
      value_type x = std::move(whole[i]);
      M_back.push(x);
      if (!M_back_folded.empty()) x = M_back_folded.top().fold(x);
      M_back_folded.push(x);
    }
  }

public:
  foldable_deque() = default;
  foldable_deque(foldable_deque const&) = default;
  foldable_deque(foldable_deque&&) = default;

  // TODO: construct from initializer list, assign(), etc.

  foldable_deque& operator =(foldable_deque const&) = default;
  foldable_deque& operator =(foldable_deque&&) = default;

  bool empty() const noexcept { return M_front.empty() && M_back.empty(); }
  size_type size() const noexcept { return M_front.size() + M_back.size(); }

  void push_back(value_type const& x) {
    M_back.push(x);
    if (M_back_folded.empty()) {
      M_back_folded.push(x);
    } else {
      value_type tmp = M_back_folded.top();
      tmp.fold_eq(x);
      M_back_folded.push(tmp);
    }
  }
  void push_front(value_type const& x) {
    M_front.push(x);
    if (M_front_folded.empty()) {
      M_front_folded.push(x);
    } else {
      value_type tmp = x;
      tmp.fold_eq(M_front_folded.top());
      M_front_folded.push(tmp);
    }
  }

  template <typename... Args>
  void emplace_back(Args&&... args) { push_back(value_type(args...)); }
  template <typename... Args>
  void emplace_front(Args&&... args) { push_front(value_type(args...)); }

  void pop_back() {
    if (M_back.empty()) M_balance();
    M_back.pop();
    M_back_folded.pop();
  }
  void pop_front() {
    if (M_front.empty()) M_balance();
    M_front.pop();
    M_front_folded.pop();
  }

  value_type fold() const {
    value_type res;
    if (!M_front_folded.empty()) res.fold_eq(M_front_folded.top());
    if (!M_back_folded.empty()) res.fold_eq(M_back_folded.top());
    return res;
  }
};
