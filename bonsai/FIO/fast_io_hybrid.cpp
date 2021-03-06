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
  static __attribute__((aligned(8))) char minibuf[8];
  static size_t constexpr int_digits = 20;  // 18446744073709551615
  static uintmax_t constexpr int_mask = 0x3030303030303030;
  static uintmax_t constexpr first_mask = 0x00FF00FF00FF00FF;
  static uintmax_t constexpr second_mask = 0x0000FFFF0000FFFF;
  static uintmax_t constexpr third_mask = 0x00000000FFFFFFFF;
  static uintmax_t constexpr tenpow[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000
  };
  template <typename Tp>
  using enable_if_integral = std::enable_if<std::is_integral<Tp>::value, Tp>;

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
              typename enable_if_integral<Integral>::type* = nullptr>
    void scan_parallel(Integral& x) {
      if (endpos <= pos + int_digits) reread_from_stdin();
      bool ends = false;
      x = 0;
      do {
        memcpy(minibuf, pos, 8);
        long c = *(long*)minibuf;
        long d = (c & int_mask) ^ int_mask;
        int skip = 8;
        int shift = 8;
        if (d) {
          int ctz = __builtin_ctzl(d);
          if (ctz == 4) break;
          c &= (1L << (ctz-5)) - 1;
          int discarded = (68-ctz) / 8;
          shift -= discarded;
          c <<= discarded * 8;
          skip -= discarded;
          ends = true;
        }
        c |= int_mask;
        c ^= int_mask;
        c = ((c >> 8) + c*10) & first_mask;
        c = ((c >> 16) + c*100) & second_mask;
        c = ((c >> 32) + c*10000) & third_mask;
        x = x*tenpow[shift] + c;
        pos += skip;
      } while (!ends);
      ++pos;
    }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    void scan_serial(Integral& x) {
      if (endpos <= pos + int_digits) reread_from_stdin();
      x = 0;
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
      ++pos;
    }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    void scan(Integral& x) { scan_serial(x); }
  };
} // fast::

fast::scanner cin;

int main() {
  intmax_t x;
  cin.scan(x);
  printf("%jd\n", x);
}
