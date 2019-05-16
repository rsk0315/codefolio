template <class Container>
class enumerated_container {
public:
  using underlying_iterator = typename Container::const_iterator;
  using value_type = typename Container::value_type;

  class iterator {
    size_t i;
    underlying_iterator it;
  public:
    iterator(size_t i, const underlying_iterator& it): i(i), it(it) {}
    std::pair<size_t, value_type> operator *() const { return {i, *it}; }
    iterator& operator ++() { ++i, ++it; return *this; }
    bool operator !=(const iterator& other) const { return it != other.it; }
  };

private:
  underlying_iterator begin_, end_;
  size_t i = 0;

public:
  enumerated_container(const Container& c): begin_(c.begin()), end_(c.end()) {}

  iterator begin() const { return {0, begin_}; }
  iterator end() const { return {0, end_}; }
};

template <class Container>
enumerated_container<Container> enumerate(const Container& c) {
  return enumerated_container<Container>(c);
}
