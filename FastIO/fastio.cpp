#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>
#include <type_traits>
#include <limits>

namespace fast {
  // fast I/O by rsk0315 (update: 2019-08-08 07:02:47).
  // This version supports only integer inputs/outputs, single character
  // outputs, and string literal outputs.
  static size_t constexpr buf_size = 1 << 17;
  static size_t constexpr margin = 1;
  static char inbuf[buf_size + margin] = {};
  static char outbuf[buf_size + margin] = {};
  static __attribute__((aligned(8))) char minibuf[32];
  static size_t constexpr int_digits = 20;  // 18446744073709551615
  static uintmax_t constexpr digit_mask = 0x3030303030303030;
  static uintmax_t constexpr first_mask = 0x00FF00FF00FF00FF;
  static uintmax_t constexpr second_mask = 0x0000FFFF0000FFFF;
  static uintmax_t constexpr third_mask = 0x00000000FFFFFFFF;
  static uintmax_t constexpr tenpow[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000
  };
  template <typename Tp>
  using enable_if_integral = std::enable_if<std::is_integral<Tp>::value, Tp>;

  class scanner {
    char* pos = inbuf;
    char* endpos = inbuf + buf_size;

    void M_read_from_stdin() {
      endpos = inbuf + fread(pos, 1, buf_size, stdin);
    }
    void M_reread_from_stdin() {
      ptrdiff_t len = endpos - pos;
      if (!(inbuf + len <= pos)) return;
      memcpy(inbuf, pos, len);
      char* tmp = inbuf + len;
      endpos = tmp + fread(tmp, 1, buf_size-len, stdin);
      *endpos = 0;
      pos = inbuf;
    }

  public:
    scanner() { M_read_from_stdin(); }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    void scan_parallel(Integral& x) {
      // See https://qiita.com/rsk0315_h4x/items/17a9cb12e0de5fd918f4
      if (__builtin_expect(endpos <= pos + int_digits, 0))
        M_reread_from_stdin();
      bool ends = false;
      typename std::make_unsigned<Integral>::type y = 0;
      bool neg = false;
      if (std::is_signed<Integral>::value && *pos == '-') {
        neg = true;
        ++pos;
      }
      do {
        memcpy(minibuf, pos, 8);
        long c = *(long*)minibuf;
        long d = (c & digit_mask) ^ digit_mask;
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
        c |= digit_mask;
        c ^= digit_mask;
        c = ((c >> 8) + c*10) & first_mask;
        c = ((c >> 16) + c*100) & second_mask;
        c = ((c >> 32) + c*10000) & third_mask;
        y = y*tenpow[shift] + c;
        pos += skip;
      } while (!ends);
      x = (neg? -y: y);
      ++pos;
    }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    void scan_serial(Integral& x) {
      if (__builtin_expect(endpos <= pos + int_digits, 0))
        M_reread_from_stdin();
      bool neg = false;
      if (std::is_signed<Integral>::value && *pos == '-') {
        neg = true;
        ++pos;
      }
      typename std::make_unsigned<Integral>::type y = *pos-'0';
      while (*++pos >= '0') y = 10*y + *pos-'0';
      x = (neg? -y: y);
      ++pos;
    }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    // Use scan_parallel(x) only when x may be too large (about 10^12).
    // Otherwise, even when x <= 10^9, scan_serial(x) may be faster.
    void scan(Integral& x) { scan_serial(x); }
  };

  class printer {
    char* pos = outbuf;

    void M_flush_stdout() {
      fwrite(outbuf, 1, pos-outbuf, stdout);
      pos = outbuf;
    }

  public:
    printer() = default;
    ~printer() { M_flush_stdout(); }

    void print(char c) {
      if (__builtin_expect(pos + 1 >= outbuf + buf_size, 0)) M_flush_stdout();
      *pos++ = c;
    }

    template <size_t N>
    void print(char const(&s)[N]) {
      if (__builtin_expect(pos + N >= outbuf + buf_size, 0)) M_flush_stdout();
      memcpy(pos, s, N-1);
      pos += N-1;
    }

    template <typename Integral,
              typename enable_if_integral<Integral>::type* = nullptr>
    void print(Integral x) {
      if (__builtin_expect(pos + int_digits >= outbuf + buf_size, 0))
        M_flush_stdout();
      if (x == 0) {
        *pos++ = '0';
        return;
      }
      if (x < 0) {
        *pos++ = '-';
        if (__builtin_expect(x == std::numeric_limits<Integral>::min(), 0)) {
          switch (sizeof x) {
          case 2: print("32768"); return;
          case 4: print("2147483648"); return;
          case 8: print("9223372036854775808"); return;
          }
        }
        x = -x;
      }
      char* mp = minibuf + sizeof minibuf;
      *--mp = x % 10 + '0';
      size_t len = 1;
      typename std::make_unsigned<Integral>::type y = x / 10;
      while (y > 0) {
        *--mp = y % 10 + '0';
        y /= 10;
        ++len;
      }
      memcpy(pos, mp, len);
      pos += len;
    }

    template <typename Tp>
    void println(Tp x) { print(x), print('\n'); }
  };
} // fast::

fast::scanner cin;
fast::printer cout;
