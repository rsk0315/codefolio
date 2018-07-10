#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>

intmax_t apply(int lhs, char op, int rhs) {
  switch (op) {
  case '+': return lhs + rhs;
  case '-': return lhs - rhs;
  case '*': return lhs * rhs;
  case '/': return lhs / rhs;
  }
  assert(false);
}

#include "./simple.cpp"

int main() {
  while (true) {
    char buf[128];
    scanf("%s", buf);
    std::string s=buf;
    size_t i=0;
    printf("%jd\n", parse(s, i));
  }
}
