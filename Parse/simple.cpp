intmax_t parse(
    const std::string &s, size_t &i,
    const std::vector<std::string> &ops={"+-", "*/"}, size_t preced=0) {

  if (preced == ops.size()) {
    if (s[i] == '(') {
      intmax_t res=parse(s, ++i, ops, 0);
      assert(s[i] == ')');
      ++i;
      return res;
    } else if (s[i] == '-') {
      return -parse(s, ++i, ops, preced);
    } else if (isdigit(s[i])) {
      intmax_t res=s[i]-'0';
      while (++i < s.length() && isdigit(s[i]))
        res = res*10+s[i]-'0';
      return res;
    }
    assert(false);
  }
  intmax_t lhs=parse(s, i, ops, preced+1);
  while (i < s.length()) {
    char op=s[i];
    if (!std::count(ops[preced].begin(), ops[preced].end(), op)) break;
    intmax_t rhs=parse(s, ++i, ops, preced+1);
    lhs = apply(lhs, op, rhs);
  }
  return lhs;
}
