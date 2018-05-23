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
  std::map<std::string, intmax_t> vars;
  char open, close;
  int max_preced;
  std::map<char, std::function<intmax_t (intmax_t, intmax_t)>> operators;

  intmax_t apply(intmax_t lhs, char op, intmax_t rhs) const {
    if (operators.find(op) != operators.end()) {
      return operators.at(op)(lhs, rhs);
    }

    switch (op) {
      case '+': return lhs + rhs;
      case '-': return lhs - rhs;
      case '*': return lhs * rhs;
      default: assert(false);
    }
  }

  intmax_t literal(
      const std::string &s, size_t &i,
      const std::map<std::string, intmax_t> &tmp_vars) const {

    if (!isdigit(s[i])) {
      std::string param(1, s[i]);
      while (i++ < s.length() && isalnum(s[i]))
        param += s[i];

      if (vars.find(param) != vars.end()) {
        return vars.at(param);
      } else {
        return tmp_vars.at(param);
      }
    }

    intmax_t res=s[i]-'0';
    while (++i < s.length() && isdigit(s[i])) {
      res = res*10 + s[i]-'0';
    }
    return res;
  }

  intmax_t parse(
      const std::string &s, size_t &i, int level,
      const std::map<std::string, intmax_t> &tmp_vars) const {

    /* tsurai parser */
    if (level == max_preced) {
      if (s[i] == open) {
        intmax_t tmp=parse(s, ++i, 0, tmp_vars);
        assert(s[i] == close);
        ++i;
        return tmp;
      }

      if (s[i] == '+') {
        return parse(s, ++i, level, tmp_vars);
      } else if (s[i] == '-') {
        // UB but maybe works with INT*_MIN...
        return -parse(s, ++i, level, tmp_vars);
      } else {
        return literal(s, i, tmp_vars);
      }
    }

    intmax_t lhs=parse(s, i, level+1, tmp_vars);
    while (i < s.length()) {
      int op=s[i];
      if (op == close || preced[op] != level) break;
      if (assoc[op] == '>') {
        intmax_t rhs=parse(s, ++i, level+1, tmp_vars);
        lhs = apply(lhs, op, rhs);
      } else /* if (assoc[op] == '<') */ {
        lhs = apply(lhs, op, parse(s, ++i, level, tmp_vars));
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

  intmax_t parse(
      const std::string &s,
      const std::map<std::string, intmax_t> &tmp_vars={}) const {

    size_t i=0;
    intmax_t res=parse(s, i, 0, tmp_vars);
    return res;
  }

  void add_var(const std::string &param, intmax_t value) {
    vars[param] = value;
  }

  void add_operator(
      char ch, int a, int p,
      std::function<intmax_t (intmax_t, intmax_t)> op) {

    assoc[ch] = a;
    preced[ch] = p;
    max_preced = std::max(max_preced, p+1);
    operators[ch] = op;
  }

  void add_operator(char ch, int a, int p, const std::string &t) {
    add_operator(ch, a, p, [&](intmax_t x, intmax_t y) {
      return this->parse(t, {{"x", x}, {"y", y}});
    });
  }
};

int main() {
  Parser p({">+-", ">*/"});
  std::map<char, std::function<intmax_t (intmax_t, intmax_t)>> callback;
  p.add_operator('^', '<', 2, [](intmax_t x, intmax_t y)->intmax_t {
    return ppow(x, y);
  });

  size_t n;
  scanf("%zu", &n);
  for (size_t i=0; i<n; ++i) {
    char op;
    char buf[96];
    scanf(" %c %s", &op, buf);
    p.add_operator(op, '>', 3, buf);
  }

  char buf[128];
  scanf("%s", buf);
  printf("%jd\n", p.parse(buf));
}
