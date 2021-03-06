## CYK[^cyk]法
### 2018年7月17日
#[^cyk]: Cocke--Younger--Kasami．

与えられた文$S$が文脈自由文法$\mathscr{G}=[T, N, \sigma, P]$でに含まれるかを$O(|P|\cdot|S|^3)$で判定するアルゴリズム．
生成規則の適用の仕方の復元も可能で，複数存在する場合はその旨を報告できる．

区間DPみたいなことをする．
DPテーブルの添字は「長さ」「開始位置」「非終端記号（のID）」で，要素は適用の方法の表す整数（true/falseでもよい）．
すなわち，$\mathrm{dp}[i][j][k]$は，$S$の位置$j$から長さ$i$の部分文字列を$k$番目の非終端記号から導出することが可能かどうか（あるいはその種類数）を表す．ここでは1-indexedで記すが，もちろん0-indexedでもできる．予め生成規則をChomsky標準形に直しておく必要がある．

まず，$S$に含まれる各文字について，その位置とそれを直接導出する生成規則を求め，それに対応する要素（長さは$1$）の値を$1$にする．
長さ$i$を$2$から順に増やしていき，その区間$[j, j+i]$を二分した区間$[j, j+k-1]$および$[j+k, j+i]$に対応する部分文字列を見る（順に$S_A$，$S_B$，$S_C$とする）．
$S_B$と$S_C$がそれぞれ非終端記号$b$と$c$から導出可能で，かつ$a\to b\,c$とするような生成規則が存在した場合，$S_A$は非終端記号$a$から導出可能であることが分かる．
すなわち$\mathrm{dp}[i][j][a]$を$\mathrm{dp}[k][j][b]$と$\mathrm{dp}[i-k][j+k][c]$から更新する．
復元したい場合は$(i, j, a)$から$(k, b, c)$に対応させる情報を持っておくとよい．

$\mathrm{dp}[|S|][1][\sigma]$が最終的な答えである．
