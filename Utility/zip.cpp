#include <cstdio>
#include <vector>
#include <tuple>
#include <utility>

template <class... Containers>
class zipped_containers {
  static constexpr auto is = std::make_index_sequence<sizeof...(Containers)>{};
  using value_type = std::tuple<typename Containers::value_type...>;
  using reference = std::tuple<typename Containers::reference...>;

  template <class... Iterators>
  class iterator_: std::tuple<Iterators...> {
    template <size_t... Is>
    void M_increment(std::index_sequence<Is...>) {
      std::make_tuple(++std::get<Is>(*this)...);
    }
    template <size_t... Is>
    reference M_dereference(std::index_sequence<Is...>) {
      return std::tie(*std::get<Is>(*this)...);
    }
    template <size_t... Is>
    bool M_not_equal(const iterator_& other, std::index_sequence<Is...>) const {
      bool ne = true;
      std::vector<bool>{(ne &= (std::get<Is>(*this) != std::get<Is>(other)))...};
      return ne;
    }

  public:
    iterator_() {}
    iterator_(const Iterators&... it): std::tuple<Iterators...>(it...) {}
    iterator_& operator ++() { M_increment(is); return *this; }
    reference operator *() {
      // fprintf(stderr, "M_dereference()\n"); 
      return M_dereference(is);
    }
    bool operator !=(const iterator_& other) const { return M_not_equal(other, is); }
  };

  using iterator = iterator_<typename Containers::iterator...>;
  iterator begin_, end_;

public:
  zipped_containers(Containers&... c): begin_(c.begin()...), end_(c.end()...) {}

  iterator begin() const { fprintf(stderr, "begin\n"); return begin_; }
  iterator end() const { return end_; }
};

template <class... Containers>
zipped_containers<Containers...> zip(Containers&... c) {
  return zipped_containers<Containers...>(c...);
}

template <class... Containers>
class const_zipped_containers {
  static constexpr auto is = std::make_index_sequence<sizeof...(Containers)>{};
  using value_type = std::tuple<typename Containers::value_type...>;
  using const_reference = std::tuple<typename Containers::const_reference...>;

  template <class... Iterators>
  class const_iterator_: std::tuple<Iterators...> {
    template <size_t... Is>
    void M_increment(std::index_sequence<Is...>) {
      std::make_tuple(++std::get<Is>(*this)...);
    }
    template <size_t... Is>
    const_reference M_const_dereference(std::index_sequence<Is...>) const {
      return std::tie(*std::get<Is>(*this)...);
    }
    template <size_t... Is>
    bool M_not_equal(const const_iterator_& other, std::index_sequence<Is...>) const {
      bool ne = true;
      std::vector<bool>{(ne &= (std::get<Is>(*this) != std::get<Is>(other)))...};
      return ne;
    }

  public:
    const_iterator_() {}
    const_iterator_(const Iterators&... it): std::tuple<Iterators...>(it...) {}
    const_iterator_& operator ++() { M_increment(is); return *this; }
    const_reference operator *() const {
      // fprintf(stderr, "M_const_dereference()\n");
      return M_const_dereference(is);
    }
    bool operator !=(const const_iterator_& other) const { return M_not_equal(other, is); }
  };

  using const_iterator = const_iterator_<typename Containers::const_iterator...>;
  const_iterator begin_, end_;

public:
  const_zipped_containers(const Containers&... c): begin_(c.cbegin()...), end_(c.cend()...) {}

  const_iterator begin() const { fprintf(stderr, "cbegin\n"); return begin_; }
  const_iterator end() const { return end_; }
};

template <class... Containers>
const_zipped_containers<Containers...> const_zip(const Containers&... c) {
  return const_zipped_containers<Containers...>(c...);
}

int main() {
  std::vector<int> x{1, 2, 3, 4};
  std::vector<double> y{1.0, 3.0, 5.0};
  std::vector<char> z{'a', 'b', 'y'};

  for (auto [xi, yi, zi]: zip(x, y, z)) {
    printf("%d %f %c\n", xi, yi, zi);
  }

  for (auto [xi, yi, zi]: zip(x, y, z)) {
    xi += 2;
    yi += 2;
    zi += 2;
  }

  for (auto [xi, yi, zi]: zip(x, y, z)) {
    printf("%d %f %c\n", xi, yi, zi);
  }

  for (auto [xi, yi, zi]: const_zip(x, y, z)) {
    // xi += 2;  // error
    printf("%d\n", xi);
  }
}
