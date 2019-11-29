template <class Monoid>
class foldable_queue {
public:
  using size_type = size_t;
  using value_type = Monoid;

private:
  std::stack<value_type> M_front, M_back;
  value_type M_back_folded;

  void M_move_to_front() {
    std::queue<value_type> tmp;
    while (!M_back.empty()) {
      tmp.push(M_back.top());
      M_back.pop();
    }
    M_front.push(tmp.front());
    tmp.pop();
    while (!tmp.empty()) {
      value_type x = std::move(tmp.front());
      tmp.pop();
      x.fold_eq(M_front.top());
      M_front.push(x);
    }
    M_back_folded = value_type{};
  }

public:
  foldable_queue() = default;
  foldable_queue(foldable_queue const&) = default;
  foldable_queue(foldable_queue&&) = default;

  foldable_queue& operator =(foldable_queue const&) = default;
  foldable_queue& operator =(foldable_queue&&) = default;

  size_type size() const noexcept { return M_front.size() + M_back.size(); }
  bool empty() const noexcept { return M_front.empty() && M_back.empty(); }

  void push(value_type const& x) {
    M_back_folded.fold_eq(x);
    M_back.push(x);
  }

  void pop() {
    if (M_front.empty()) M_move_to_front();
    M_front.pop();
  }

  value_type fold() const {
    if (M_front.empty()) return M_back_folded;
    return M_front.top().fold(M_back_folded);
  }
};
