template <typename Fn, typename... Args>
class cached_function {
public:
  using result_type = decltype(std::declval<Fn>()(std::declval<Args>()...));
  using function_type = Fn;

private:
  using argument_tuple = std::tuple<Args...>;
  std::map<argument_tuple, result_type> M_c;
  function_type M_f;

public:
  cached_function(function_type f): M_f(f) {}

  result_type operator ()(Args&&... args) {
    auto t = std::make_tuple(args...);
    auto it = M_c.find(t);
    if (it != M_c.end()) return it->second;
    auto rv = M_f(std::forward<Args...>(args...));
    M_c.emplace(t, rv);
    return rv;
  }
};

template <typename Rt, typename... Args>
class arguments {
  using type = Args;
};

template <typename Fn, typename... Args>
auto make_cached_function() {
  return cached_function<Fn, Args...>(Fn());
}
template <typename Fn>
auto make_cached_function(Fn f) {
  return cached_function(&f.operator ());
}
template <typename Rt, typename... Args>
auto make_cached_function(Rt (*f)(Args...)) {
  return cached_function<Rt (*)(Args...), Args...>(f);
}

struct func {
  int operator ()(int x) const {
    fprintf(stderr, "first call: f(%d)\n", x);
    return x+1;
  }
};

int g(int x) {
  fprintf(stderr, "first call: g(%d)\n", x);
  return x+1;
}
