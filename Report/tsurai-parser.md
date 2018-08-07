## tsurai Parser
### 2018年8月7日

四則演算を処理する構文解析器．
名前は弊学の競プロサークルの通称から．

いわゆる`<term>`と`<factor>`において書くべき処理はほぼ同じなので，演算子の優先度を引数として渡して処理する．
演算子は優先度ごとに分けて持っておく．
擬似コードは以下の通り．

#`
const Vector<String> operators={"+-", "*/"};

ResultType parse(String s, size_t &i, size_t precedence):
  if precedence is operators.size():
    if s[i] is an opening parenthesis:
       // "(" <expr> ")"
       ResultType result=parse(s, ++i, 0);
       assert s[i] is a closing parenthesis;
       ++i;
       return result;

     if s[i] is a digit:
       ResultType result=s[i]-'0';
       while ++i < s.length() and s[i] is a digit:
         result = result*10+s[i]-'0';

       return result;

     if s[i] is a unary operator:
       if s[i] == '+': return +parse(s, ++i, precedence)
       if s[i] == '-': return -parse(s, ++i, precedence)

  Result result=parse(s, i, precedence+1);
  while i < s.length():
    char op=s[i];
    if op not in operators[precedence]: break
    Result tmp=parse(s, ++i, precedence+1);
    result = apply(result, op, tmp);

  return result;
#`

単項演算子の優先度が固定されているとか，上の例では二項演算子が左結合と仮定しているとか，不完全ではある．