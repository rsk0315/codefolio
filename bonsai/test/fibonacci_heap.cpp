#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <climits>
#include <algorithm>
#include <memory>
#include <utility>
#include <list>
#include <vector>

#include "../../utility/literals.cpp"
#include "../../utility/bit.cpp"

#include "../FH/fibonacci_heap.cpp"

int main() {
  fibonacci_heap<int, char> fh;
  fh.push(1, 'b');
  printf("%d %c\n", fh.top().first, fh.top().second);
  fh.push(2, 'a');
  printf("%d %c\n", fh.top().first, fh.top().second);
  fh.push(1, 'c');
  printf("%d %c\n", fh.top().first, fh.top().second);
  fh.pop();
  printf("%d %c\n", fh.top().first, fh.top().second);
}
