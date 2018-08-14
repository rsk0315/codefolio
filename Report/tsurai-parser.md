## tsurai Parser
### 2018年8月7日
### 2018年8月15日（編集）

四則演算を処理する構文解析器．
名前は弊学の競プロサークルの通称から．

いわゆる`<term>`と`<factor>`において書くべき処理はほぼ同じなので，演算子の優先度を引数として渡して処理する．
演算子は優先度ごとに分けて持っておく．
- $\langle\mathrm{expr}_0\rangle\coloneqq\langle\mathrm{expr}_1\rangle\mid\langle\mathrm{expr}_0\rangle\langle\mathrm{bop}_1\rangle\langle\mathrm{expr}_1\rangle$
- $\langle\mathrm{expr}_1\rangle\coloneqq\langle\mathrm{expr}_2\rangle\mid\langle\mathrm{expr}_1\rangle\langle\mathrm{bop}_2\rangle\langle\mathrm{expr}_2\rangle$
- $\quad\vdots$
- $\langle\mathrm{expr}_N\rangle\coloneqq\langle\mathrm{literal}\rangle\mid\text{ \textlt{"\textmt{(}"} }\langle\mathrm{expr}_0\rangle\text{ \textlt{"\textmt{)}"}}\mid\langle\mathrm{uop}\rangle\langle\mathrm{expr}_N\rangle$

対応するのは上のような文法で，$\langle\mathrm{bop}_i\rangle$は二項演算子，$\langle\mathrm{uop}\rangle$は単項演算子にである．
優先度の異なる単項演算子が出てくる場合はもう少し工夫が必要．
擬似コード的なものを以下に示す[^pseudo-code]．
#[^pseudo-code]: `algorithm`的な環境を使うべきでは？

通常の算術においては，$\langle\mathrm{bop}_0\rangle$は`+`または`-`，$\langle\mathrm{bop}_1\rangle$は`*`または`/`を直接導出する非終端記号という扱いになる．

- `parse`$(s, i, j)$
  - *if* $j = N$ *then*
    - *if* $s_i = \text{\textlt{"\textmt{(}"}}$
      - $a \gets \text{\texttt{parse}}(s, \text{\texttt{++}}i, 0)$
      - *assert* $s_i = \text{\textlt{"\textmt{)}"}}$
      - `++`$i$
      - *return* $a$
    - *if* $s_i$ is digit *then*
        - parse an integer and return it
    - *if* $s_i \in \langle\mathrm{uop}\rangle$ *then*
      - $\oplus \gets s_i$
      - *return* $\oplus\text{\texttt{parse}}(s, \text{\texttt{++}}i, N)$
    - *unreachable*
  - $a \gets \text{\texttt{parse}}(s, i, j+1)$
  - *while* $i \le |s|$ *do*
    - $\oplus \gets s_i$
    - *if* $\oplus \notin \langle\mathrm{bop}_j\rangle$ *then* *break*
    - $a \gets a \oplus \text{\texttt{parse}}(s, \text{\texttt{++}}i, j+1)$
  - *return* $a$

上の擬似コードでは$i$が1-indexedなので注意[^indexing]．
#[^indexing]: 擬似コードでは添字は$1$から始めたくなりがち．非終端記号は$0$からなんだけど．
右結合な演算子の場合は，下から二行目を$a \gets a \oplus \text{\texttt{parse}}(s, \text{\texttt{++}}i, j)$にするといい気がする．
本当かなぁ．
