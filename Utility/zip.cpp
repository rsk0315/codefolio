#include <cstdio>
#include <vector>
#include <tuple>
#include <utility>

template <class Fn, class... Els, size_t... Is>
void transform_tuple(std::tuple<Els...>& t, Fn&& f, std::index_sequence<Is...>) {
  std::make_tuple((f(std::get<Is>(t)), 0)...);
}

template <class Fn, class... Els, size_t... Is>
auto map_tuple(const std::tuple<Els...>& t, Fn&& f, std::index_sequence<Is...>) {
  return std::make_tuple(f(std::get<Is>(t))...);
}

template <class Fn, class... Els, size_t... Is>
void compare_tuple(const std::tuple<Els...>& t, const std::tuple<Els...>& u,
                   Fn&& f, std::index_sequence<Is...>) {

  std::make_tuple((f(std::get<Is>(t), std::get<Is>(u)), 0)...);
}

template <class... Containers>
class zipped_containers {
  static constexpr auto is = std::make_index_sequence<sizeof...(Containers)>{};
  using value_type = decltype(std::make_tuple((*Containers().cbegin())...));

  template <class... Iterators>
  class iterator_: std::tuple<Iterators...> {
  public:
    iterator_() {}
    iterator_(const Iterators&... it): std::tuple<Iterators...>(it...) {}
    iterator_& operator ++() {
      transform_tuple(*this, [](auto& it) { ++it; }, is);
      return *this;
    }
    value_type operator *() const {
      return map_tuple(*this, [](auto& it) { return *it; }, is);
    }
    bool operator !=(const iterator_& other) const {
      bool ne = true;
      compare_tuple(*this, other, [&ne](const auto il, const auto ir) { ne &= (il != ir); }, is);
      return ne;
    }
  };
  using iterator = iterator_<decltype(Containers().cbegin())...>;
  iterator begin_, end_;

public:
  zipped_containers(const Containers&... c):
    begin_(c.begin()...), end_(c.end()...)
  {}

  iterator begin() const { return begin_; }
  iterator end() const { return end_; }  
};

template <class... Containers>
zipped_containers<Containers...> zip(const Containers&... c) {
  return zipped_containers<Containers...>(c...);
}

int main() {
  std::vector<int> x{1, 2, 3, 4};
  std::vector<double> y{1.0, 3.0, 5.0};
  std::vector<char> z{'a', 'b', 'z'};

  zip(x, y, z);

  for (const auto& p: zip(x, y, z)) {
    int x;
    double y;
    char z;
    std::tie(x, y, z) = p;
    printf("%d %f %c\n", x, y, z);
  }
}
