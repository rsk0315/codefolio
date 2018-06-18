#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

int literal(const std::string &s, size_t &i) {
  assert(i < s.length());
  int res=s[i]-'0';
  while (++i < s.length() && isdigit(s[i]))
    res = res*10 + s[i]-'0';

  return res;
}

int apply(int lhs, int op, int rhs) {
  switch (op) {
    case '+': return lhs+rhs;
    case '-': return lhs-rhs;
    case '*': return lhs*rhs;
    case '/': return lhs/rhs;
    case '%': return lhs%rhs;
    default: assert(false); 
  }
}

// 1+2*3^4*5+6+7
// ..--===--....

int parse(
    const std::string &s, size_t &i, const std::vector<int> prec,
    const std::string paren="()", int lv=0) {

  // e.g. prec['+'] = 0, prec['*'] = 1, etc.
  fprintf(stderr, "%*c\n", int(i+1), '^');
  assert(i < s.length());
  if (s[i] == paren[0]) {
    int res=parse(s, ++i, prec, paren, 0);
    assert(s[i] == paren[1]);
    ++i;
    return res;
  }

  assert(isdigit(s[i]));
  int lhs=literal(s, i);
  while (i < s.length() && s[i] != paren[1]) {
    int op=s[i];  // to avoid [-Wchar-subscripts]
    if (prec[op] < lv) return lhs;

    if (prec[op] > lv) {
      int rlhs=literal(s, ++i);
      if (i == s.length()) 
        return apply(lhs, op, rlhs);

      int rop=s[i];
      if (lv > prec[rop]) return apply(lhs, op, rlhs);

      int rrhs=parse(s, ++i, prec, paren, prec[rop]);
      int rhs=apply(rlhs, rop, rrhs);
      lhs = apply(lhs, op, rhs);
    } else {
      int rhs=parse(s, ++i, prec, paren, lv+1);
      lhs = apply(lhs, op, rhs);
    }
    fprintf(stderr, "lhs: %d\n", lhs);
  }
  return lhs;
}

int fourt(
    const std::string &s,
    const std::vector<std::string> &ops={"+-", "*/%"},
    const std::string &paren="()") {

  std::vector<int> prec(128);
  for (size_t i=0; i<ops.size(); ++i)
    for (char op: ops[i])
      prec[op] = i;

  size_t i=0;
  return parse(s, i, prec, paren, 0);
}

int main() {
  char buf[256];
  scanf("%s", buf);
  std::string s=buf;
  printf("%d\n", fourt(s));
}
      
