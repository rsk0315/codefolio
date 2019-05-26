#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <type_traits>
#include <limits>
#include <tuple>
#include <deque>

namespace fast {
  static size_t constexpr buf_size = 1 << 17;
  static size_t constexpr margin = 1;
  static char buf[buf_size + margin] = {};
  static size_t constexpr int_digits = 20;  // 18446744073709551615

  class scanner {
    char* pos = buf;
    char* endpos = buf + buf_size;

    void read_from_stdin() {
      endpos = buf + fread(pos, 1, buf_size, stdin);
    }
    void reread_from_stdin() {
      ptrdiff_t cur_len = endpos - pos;
      if (!(buf + cur_len <= pos)) return;
      memcpy(buf, pos, cur_len);
      char* tmp = buf + cur_len;
      endpos = tmp + fread(tmp, 1, buf_size - cur_len, stdin);
      *endpos = 0;
      pos = buf;
    }

  public:
    scanner() { read_from_stdin(); }

    template <typename Integral,
              typename std::enable_if<std::is_integral<Integral>::value, Integral>::type* = nullptr>
    void scan(Integral& x) {
      if (endpos <= pos + int_digits) reread_from_stdin();
      x = 0;
      asm ("nya:");
      if (!std::is_signed<Integral>::value) {
        // no need to take care of overflow
        x = *pos-'0';
        while (*++pos >= '0') x = 10*x + *pos-'0';
      } else {
        bool neg = false;
        if (*pos == '-') {
          neg = true;
          ++pos;
        }
        typename std::make_unsigned<Integral>::type y = *pos-'0';
        while (*++pos >= '0') y = 10*y + *pos-'0';
        x = (neg? -y:y);
      }
      asm ("nyan:");
      ++pos;
    }
  };
} // fast::

fast::scanner cin;

int main() {
  size_t s, u;
  cin.scan(s);
  cin.scan(u);
  std::vector<signed> a(s);
  for (auto& ai: a) cin.scan(ai);
  std::vector<unsigned> b(u);
  for (auto& bi: b) cin.scan(bi);

  for (auto ai: a) printf("%d\n", ai);
  for (auto bi: b) printf("%u\n", bi);
}
