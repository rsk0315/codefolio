#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cinttypes>

#include <type_traits>
#include <limits>

/* FastIO by rsk0315 (last update: 2018/02/21 00:36) */

namespace FastIn {
  static constexpr size_t BUF_SIZE=1<<17, INT_LEN=24, FLT_LEN=400;
  static char buf[BUF_SIZE|1]={}, *pos=buf, *endbuf=nullptr;
  FILE *fin;

  inline bool rebuffer() {
    // returns true <=> there is at least one unread character
    size_t rest=endbuf-pos;
    if (buf+rest > pos) {
      // buf[:pos] and buf[-pos:] are overlapping, which std::memcpy()
      // causes undefined behavior.
      return true;
    }

    std::memcpy(buf, pos, rest); 
    pos = buf;
    size_t len=std::fread(pos+rest, 1, BUF_SIZE-rest, fin);
    *(endbuf = buf + (rest+len)) = 0;

    return *pos;
  }

  inline bool scan(char &in) {
    if ((in = *pos)) {
      ++pos;
      return true;
    }

    return rebuffer() && (in = *pos++);
  }

  inline bool scan(char *in) {
    if ((*in = *pos) == 0) {
      if (rebuffer() && (*in = *pos) == 0) {
        return false;
      }
    }
    ++in;
    while (true) {
      if ((*in = *pos++) == 0) {
        if (rebuffer() && (*in = *pos++) == 0) {
          return true;
        }
      }
      ++in;
    }
  }

  inline bool scan(double &in) {
    if (pos + FLT_LEN >= endbuf && !rebuffer()) {
      in = 0.0;
      return false;
    }

    char *tmp;
    in = std::strtod(pos, &tmp);
    pos = tmp;
    return true;
  }

  template <class Int>
  inline bool scan(Int &in) {
    in = 0;

    // assume that no redundant whitespace appears
    if (pos + INT_LEN >= endbuf && !rebuffer()) {
      return false;
    }

    if (std::is_signed<Int>::value) {
      if (*pos == '-') {
        in = ~*++pos+'1';
        while (*++pos >= '0') {
          in = in*10 + ~*pos+'1';
        }
        ++pos;
        return true;
      }
    }

    // assume that numbers are separated by the character whose value is
    // less than '0' (e.g. whitespaces, newlines)
    do {
      in = in*10 + *pos-'0';
    } while (*++pos >= '0');
    ++pos;
    return true;
  }

  inline bool eat() {
    if (*pos > ' ') {
      return true;
    }

    do {
      if (*pos == 0 && !rebuffer()) {
        return false;
      }
    } while (*++pos <= ' ');

    return true;
  }

  inline bool eat(char ch) {
    if (*pos == ch) {
      return true;
    }

    do {
      if (*pos == 0 && !rebuffer()) {
        return false;
      }
    } while (*++pos != ch);

    return true;
  }

  class Scanner {
    bool rebuffer() {
      return FastIn::rebuffer();
    }

  public:
    Scanner(FILE *fin=stdin) {
      FastIn::fin = fin;
      endbuf = pos + std::fread(buf, 1, BUF_SIZE, fin);
    }

    template <class T>
    inline bool scan(T &in) {
      return FastIn::scan(in);
    }

    template <class First, class... Rest>
    inline bool scan(First &in, Rest &...ins) {
      return scan(in) && scan(ins...);
    }
  };
}

namespace FastOut {
  static constexpr size_t BUF_SIZE=1<<17, INT_LEN=24, FLT_LEN=400;
  static constexpr char FLT_FMT[]="%.16f";
  static char buf[BUF_SIZE|1]={}, *pos=buf, *endbuf=pos+BUF_SIZE;
  FILE *fout;

  inline void flush() {
    std::fwrite(buf, 1, pos-buf, fout);
    pos = buf;
  }

  inline void print(const char out) {
    if (pos == endbuf) {
      flush();
    }
    *pos++ = out;
  }

  inline void print(const char *out) {
    size_t len=std::strlen(out);
    if (pos + len >= endbuf) {
      flush();
      if (len >= BUF_SIZE) {
        std::fwrite(out, 1, len, fout);
        return;
      }
    }
    std::memcpy(pos, out, len);
    pos += len;
  }

  inline void print(char *out) {
    size_t len=std::strlen(out);
    if (pos + len >= endbuf) {
      flush();
      if (len >= BUF_SIZE) {
        std::fwrite(out, 1, len, fout);
        return;
      }
    }
    std::memcpy(pos, out, len);
    pos += len;
  }


  template <class Char, size_t Len>
  inline void print(const Char (&out)[Len]) {
    print(&out[0]);
  }

  inline void print(const double out) {
    char minibuf[FLT_LEN];
    size_t len=std::snprintf(minibuf, FLT_LEN, FLT_FMT, out);
    if (pos + len >= endbuf) {
      flush();
    }
    std::memcpy(pos, minibuf, len);
    pos += len;
  }

  inline void print(const bool out) {
    print(out? "true":"false");
  }

  template <class Int>
  inline void print(Int out) {
    static_assert(std::is_integral<Int>::value, "For integers only");

    if (out == 0) {
      if (pos == endbuf) {
        flush();
      }
      *pos++ = '0';
      return;
    }

    char minibuf[INT_LEN], *minipos=minibuf+INT_LEN;
    if (std::is_signed<Int>::value && out < 0) {
      if (pos == endbuf) {
        flush();
      }
      *pos++ = '-';
      if (out == std::numeric_limits<Int>::min() && Int(-1) == ~Int(0)) {
        // In two's complement representation, we could not represent the
        // absolute value of minimum (maximum-magnitude negative) value.
        // Without this branch, we would overflow (undefined behavior) by
        // the value.  If we know the input is never the value, we can
        // remove this branch and save (nanoseconds-order) time.
        switch (sizeof out) {
        case 1:
          return (void)(print("128"));
        case 2:
          return (void)(print("32768"));
        case 3:
          return (void)(print("8388608"));
        case 4:
          return (void)(print("2147483648"));
        case 8:
          return (void)(print("9223372036854775808"));
        case 16:
          return (void)(print("170141183460469231731687303715884105728"));
        default:
          // Regardless of the number of its bytes, lowest (decimal) digit
          // is always eight, but other digits depend on it.
          *--minipos = '8';
          out /= -10;
        }
      } else {
        out = -out;
      }
    }

    // We know the division is too slow; we wish we could avoid using them
    // TWICE PER LOOP!  ...Now, we notice that compilers are so smart that
    // can replace it by shift operations...  The division-compatible
    //instructions for unsigned are shorter than ones for signed.
    typename std::make_unsigned<Int>::type out_=out;
    do {
      *--minipos = '0' + out_%10;
      out_ /= 10;
    } while (out_ > 0);

    size_t len=(minibuf+INT_LEN)-minipos;
    if (pos + len >= endbuf) {
      flush();
    }
    std::memcpy(pos, minipos, len);
    pos += len;
  }

  class Printer {
    inline void flush() {
      FastOut::flush();
    }

  public:
    Printer(FILE *fout=stdout) {
      FastOut::fout = fout;
    }

    ~Printer() {
      flush();
    }

    template <class T>
    inline void print(const T out) {
      FastOut::print(out);
    }

    template <class First, class... Rest>
    inline void print(const First &out, const Rest &...outs) {
      FastOut::print(out);
      Printer::print(outs...);
    }

    template <class T>
    inline void println(const T out) {
      FastOut::print(out);
      FastOut::print('\n');
    }
  };
}

