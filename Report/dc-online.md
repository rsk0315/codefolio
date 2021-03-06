## Dynamic Connectivity (online)
### 2018年7月10日

Union-findにdeleteを追加したもの．"*This is a _much_ harder problem!*"[^n575]
#[^n575]: だいなみく・こねくちびちー・ぷろぶれむ

まず森に関するD.C.を解く方法を考え，それを用いて一般のグラフに関するD.C.を解く．

### 2018年7月18日
以下では，上記の森に対するD.C.を解く方法を示す．

まず，赤黒木を用意する．これは辺$(u, v)$を要素として持ち，要素をin-orderで読んだ辺の列がEuler tourを表す．
処理の都合上，自己辺$(u, u)$を各頂点に張っておく．
赤黒木自体の理解が大変なため，すでに一週間ほど使っていてつらいが，2-3-4木に助けてもらいながらがんばった．

ここでの赤黒木は`split`と`merge`をできる必要がある．
また，値の大小関係で順を管理するのではないことに注意が必要である．
`split`の際には，基準となる点から根に向かって辿り，その方向に従って左右の木にくっつけていく方法を採用した．

はじめに`reroot`$(u)$と呼ばれるサブルーチンを定義しておく．
これはEuler tourの始点を変更する処理で，以下のようにする．
- $(u, u)$で`split`し，木$L$と木$R$に分ける
- 木$R$が木$L$の左に来るように`merge`する
  - 木$R$と木$L$の間には$(u, u)$が入るようにする


`link`$(u, v)$をする際，事前条件として$u$と$v$が既につながっていないことが求められる[^dc_prereq]．
#[^dc_prereq]: これは結構つらくて，この条件下でverifyできるD.C.の問題を探すのに苦労する．そもそも存在するの？
頂点$u$，$v$を表す赤黒木のノード（すなわち$(u, u)$と$(v, v)$）が属する赤黒木をそれぞれ$T_u$，$T_v$とおく．
- $T_u$と$T_v$をそれぞれ$u$と$v$で`reroot`
- $T_u$と$T_v$を$(u, v)$をはさんで`merge`
- $T_v$の末尾に$(v, u)$を`insert`
  - 実装によっては$T_v$と木$\{(v, u)\}$を`merge`と考えてもよい


`are-connected`$(u, v)$に答える際は，$T_u$と$T_v$の根が同じかどうかを見ればよい．

`cut`$(u, v)$の事前条件は$u$と$v$が連結していることだが，これは当たり前といえば当たり前（切れているのに切ってほしいならno-opでよい）．
- `reroot`$(u)$をして$(u, v)$の方が$(v, u)$より先に来るようにする
  - 逆でもよいが，どちらが先に来るかをはっきりさせておくと楽になる
    - これは嘘で，`reroot`$(u)$では$(u, v)$が$(v, u)$より先に来ることが保証できないよね
    - これも嘘で，全体が木であることを考えるとそれを保証できる
- $(u, v)$，$(v, u)$でそれぞれ`split`し，得られる木を$T_J$，$T_K$，$T_L$とする．
- $T_J$と$T_L$を`merge`した木が新たな$T_u$，$T_K$が新たな$T_v$となる．


以上で，森に関するD.C.は解決．

### 2018年7月31日
一般のグラフ$G$でのD.C.では，辺集合を二つに分けて考える．
$G$上で全域森（当然一意でないこともあるが，適当に決める）をなす辺集合を_tree edge_，その補集合を_regular edge_（または_non-tree edge_）とする．
また，各辺には_specificity_（または_level_）と呼ばれる非負整数値が定められていて，specificityが$k$以上のtree edgeで作られる森を$\mathscr{F}_k$とする．
この森を表現するために，上で紹介したデータ構造を用いる[^forest_dc]．
#[^forest_dc]: 森に関するD.C.を処理できるデータ構造ならなんでもいいはず．

`are-connected`$(u, v)$は簡単で，$\mathscr{F}_0$を見ればよい．$G$上での連結性は$\mathscr{F}_0$上での連結性と等価なため．

`link`$(u, v)$の手順は以下の通り．新たに辺を追加する際のspecificityの初期値は$0$である．
- $u$と$v$がすでに連結なら，regular edgeに$(u, v)$を追加して終了
- そうでない場合
  - $u$の属する木$T_u$と$v$の属する木$T_v$を連結する
  - tree edgeに$(u, v)$を追加する

`cut`$(u, v)$の手順は以下の通り．
$(u, v)$がregular edgeなら，それを取り除いて終了．
そうでなければ，ループ変数$k$の初期値を$(u, v)$のspecifityとし，$k\ge 0$の間，以下を繰り返す．
- $(u, v)$を取り除き，$u$，$v$の属するspecificityが$k$の木をそれぞれ$T_u$，$T_v$とする
  - $T_u$の頂点数の方が小さくなるように適宜入れ替える（cf. マージテク）
- $T_u$の各辺のspecificityを$1$増やす
- $T_u$から出ているregular edgeを順に見る
  - それが$T_u$と$T_v$を結んでいれば，それらの木を連結させて終了
  - そうでなければ，その辺のspecificityを$1$増やす
- 連結させられなければ，$k$を$1$減らす

最後まで連結させられなければ，実際に$u$と$v$の間のパスが消えたことになる．
各ステップを高速に処理するため，以下の対応づけを管理しておくとよい．
- $(u, v) \mapsto \text{specificity of }(u, v)$
- $u \mapsto \{\text{tree edges incident from }u\}$
- $u \mapsto \{\text{regular edges incident from }u\}$


これはHolm, de LichtenbergおよびThroupによるonlineなアルゴリズムで，$\log$がたくさんついていて重い．
CodeforcesのGym/100551-Aの制約（頂点数，クエリ数ともに最大$3\times 10^5$）には敵わなかった[^log_tle]．
#[^log_tle]: Time limit exceeded on test 15，実質ACでは？

これとは別で，offlineのアルゴリズムで$O(\log n)$のものがあるので，今後それを勉強しよう．
