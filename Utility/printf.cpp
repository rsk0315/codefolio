#include <cstring>
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>

class formatter {
  const char* fmt;
  size_t num_vars = 0;

  template <class Tp>
  void print(const char* pos, const Tp& t) const {
    const char* next = strchr(pos, '{');
    assert(next);
    std::cout << std::string(pos, next) << t;
    assert(next[1] == '}');
    std::cout << std::string(next+2);
  }

  template <class Tp, class... Ts>
  void print(const char* pos, const Tp& t, const Ts&... ts) const {
    const char* next = strchr(pos, '{');
    assert(next);
    std::cout << std::string(pos, next) << t;
    assert(next[1] == '}');
    print(next+2, ts...);
  }

public:
  constexpr formatter(const char* fmt): fmt(fmt) {
    for (size_t i = 1; fmt[i]; ++i)
      if (fmt[i-1] == '{' && fmt[i] == '}')
        ++num_vars;
  }

  template <class... Ts>
  void print(const Ts&... ts) const {
    const char* pos = fmt;
    print(pos, ts...);
  }

  constexpr size_t vars() const { return num_vars; }
};

constexpr formatter operator ""_fmt(const char* fmt, size_t) {
  return formatter(fmt);
}

#define LEN_ARGS(...)                                                   \
  std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value        \
  
#define printf_macro(fmt, ...) do {                     \
    constexpr formatter f = fmt ## _fmt;                \
    constexpr size_t lhs = f.vars();                    \
    constexpr size_t rhs = LEN_ARGS(__VA_ARGS__);       \
    static_assert(lhs == rhs);                          \
    f.print(__VA_ARGS__);                               \
  } while (false)

int main() {
  int a = 0;
  printf_macro("{} {}\n", ++a, 4.2);
}
