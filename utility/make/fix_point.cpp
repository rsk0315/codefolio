template <typename F>
class fix_point: F {
public:
  explicit constexpr fix_point(F&& f) noexcept: F(std::forward<F>(f)) {}

  template <typename... Args>
  constexpr decltype(auto) operator ()(Args&&... args) const {
    return F::operator ()(*this, std::forward<Args>(args)...);
  }
};

template <typename F>
static inline constexpr decltype(auto) make_fix_point(F&& f) noexcept {
  return fix_point<F>{std::forward<F>(f)};
}
