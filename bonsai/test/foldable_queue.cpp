#include <cstdio>
#include <cstdint>
#include <stack>
#include <vector>
#include <utility>
#include <queue>
#include <string>

#include "../SWAG/foldable_queue.cpp"

template <typename Tp>
class type_with_plus {
public:
  using value_type = Tp;
  value_type identity{};

private:
  value_type M_x = identity;

public:
  type_with_plus() = default;
  type_with_plus(type_with_plus const&) = default;
  type_with_plus(type_with_plus&&) = default;

  type_with_plus(value_type const& x): M_x(x) {}

  type_with_plus& operator =(type_with_plus const&) = default;
  type_with_plus& operator =(type_with_plus&&) = default;

  type_with_plus& operator =(value_type const& x) {
    M_x = x;
  }

  type_with_plus& fold_eq(type_with_plus const& other) {
    M_x += other.M_x;
    return *this;
  }
  type_with_plus fold(type_with_plus const& other) const {
    return type_with_plus(*this).fold_eq(other);
  }

  value_type get() const { return M_x; }
};

int main() {
  size_t n;
  scanf("%zu", &n);

  foldable_queue<type_with_plus<std::string>> fq;
  for (size_t i = 0; i < n; ++i) {
    char t;
    scanf(" %c", &t);

    if (t == '+') {
      char buf[1024];
      scanf("%s", buf);
      std::string s = buf;
      fq.push(s);
    } else if (t == '-') {
      fq.pop();
    }

    printf("%s\n", fq.fold().get().c_str());
  }
}
