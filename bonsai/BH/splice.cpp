#include <iostream>
#include <list>

template <class Tp>
std::ostream& operator <<(std::ostream& os, const std::list<Tp>& ls) {
  os << ":";
  for (const auto& x: ls) os << ' ' << x;
  return os;
}

int main() {
  std::list<int> a{1, 2, 3, 4, 5, 6};
  std::list<int> b{10, 20, 30};

  std::cout << a << std::endl;
  std::cout << b << std::endl;

  auto pos = std::next(a.begin(), 2);
  auto it = std::next(b.begin());
  a.splice(pos, b, it);

  std::cout << a << std::endl;
  std::cout << b << std::endl;

  auto first = std::next(a.begin(), 4);
  auto last = std::next(a.begin(), 6);

  a.splice(pos, a, first, last);
  std::cout << a << std::endl;

  std::list<int> c;
  it = std::next(a.begin(), 4);
  c.splice(c.begin(), a, it);

  std::cout << a << std::endl;
  std::cout << c << std::endl;
}
