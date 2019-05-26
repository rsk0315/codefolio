#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <set>

namespace fast {
  static size_t constexpr buf_size = 1 << 17;
  static size_t constexpr margin = 1;
  static char buf[buf_size + margin] = {};
  static size_t constexpr int_digits = 11;  // FIXME: impl-def

  class scanner {
    char* pos = buf;
    char* endpos = buf + buf_size;

    void read_from_stdin() {
      fread(pos, 1, buf_size, stdin);
    }
    void reread_from_stdin() {
      ptrdiff_t cur_len = endpos - pos;
      if (!(buf + cur_len <= pos)) return;
      memcpy(buf, pos, cur_len);
      char* tmp = buf + cur_len;
      endpos = tmp + fread(tmp, 1, buf_size - cur_len, stdin);
      pos = buf;
    }

  public:
    scanner() { read_from_stdin(); }

    void scan(int& x) {
      if (endpos <= pos + int_digits) reread_from_stdin();
      x = 0;
      if (*pos == '-') {
        ++pos;
        x = -*pos+'0';
        while (*++pos >= '0') x = 10*x-*pos+'0';  // XXX: ub
      } else {
        x = *pos-'0';
        while (*++pos >= '0') x = 10*x+*pos-'0';  // XXX: ub
      }
      ++pos;
    }
  };
} // fast::

fast::scanner cin;

int main() {
  int n;
  cin.scan(n);
  // scanf("%d", &n);

  std::set<int> x;
  for (int i = 0; i < n; ++i) {
    int a;
    cin.scan(a);
    // scanf("%d", &a);
    a >>= __builtin_ctz(a);
    x.insert(a);
  }
  printf("%zu\n", x.size());
}
