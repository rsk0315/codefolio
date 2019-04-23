#include <cstdio>

struct tsurai {
  int n;
  tsurai(int n): n(n+1) {
    printf("%d\n", n);
  }
};

int main() {
  tsurai tz(1);
}
