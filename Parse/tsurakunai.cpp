#include <cstdio>
#include <cstdint>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <functional>

intmax_t ppow(intmax_t base, intmax_t iexp) {
  intmax_t res=1;
  for (intmax_t dbl=base; iexp; iexp>>=1) {
    if (iexp & 1) res *= dbl;
    dbl *= dbl;
  }
  return res;
}

class Parser {
  std::vector<int> assoc, preced;
  char open, close;
  int max_preced;

  intmax_t apply(intmax_t lhs, char op, intmax_t rhs) const {
    switch (op) {
      case '+': return lhs + rhs;
      case '-': return lhs - rhs;
      case '*': return lhs * rhs;
      default: assert(false);
    }
  }

  intmax_t literal(const std::string &s, size_t &i) const {
    intmax_t res=s[i]-'0';
    while (++i < s.length() && isdigit(s[i])) {
      res = res*10 + s[i]-'0';
    }
    return res;
  }

  intmax_t parse(const std::string &s, size_t &i, int level) const {
    /* tsurai parser */
    if (level == max_preced) {
      if (s[i] == open) {
        intmax_t tmp=parse(s, ++i, 0);
        assert(s[i] == close);
        ++i;
        return tmp;
      }

      if (s[i] == '+') {
        return parse(s, ++i, level);
      } else if (s[i] == '-') {
        // UB but maybe works with INT*_MIN...
        return -parse(s, ++i, level);
      } else {
        return literal(s, i);
      }
    }

    intmax_t lhs=parse(s, i, level+1);
    while (i < s.length()) {
      int op=s[i];
      if (op == close || preced[op] != level) break;
      if (assoc[op] == '>') {
        intmax_t rhs=parse(s, ++i, level+1);
        lhs = apply(lhs, op, rhs);
      } else /* if (assoc[op] == '<') */ {
        lhs = apply(lhs, op, parse(s, ++i, level));
      }
    }
    return lhs;
  }

public:
  Parser(const std::vector<std::string> &opss, const std::string &paren="()"):
      assoc(128), preced(128), open(paren[0]), close(paren[1]),
      max_preced(opss.size())
  {
    for (size_t i=0; i<opss.size(); ++i) {
      int a=opss[i][0];  // '<' or '>'; means associativity
      for (size_t j=1; j<opss[i].size(); ++j) {
        int op=opss[i][j];
        assoc[op] = a;
        preced[op] = i;
      }
    }
  }

  intmax_t parse(const std::string &s) const {
    size_t i=0;
    intmax_t res=parse(s, i, 0);
    return res;
  }
};
