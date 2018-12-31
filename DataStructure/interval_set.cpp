template <class Tp>
class interval_set {
  // [lb, ub) の区間の集合を表します．各区間で共通部分が出ないように
  // 適宜マージしていきます．
  std::set<std::pair<Tp, Tp>> ranges;
 
public:
  void insert(Tp x) {
    insert(x, x+1);
  }
 
  void erase(Tp x) {
    erase(x, x+1);
  }
 
  void insert(Tp lb, Tp ub) {
    if (ranges.empty()) {
      ranges.emplace(lb, ub);
      return;
    }
 
    auto it=ranges.upper_bound({lb, lb});
 
    if (it != ranges.begin() && lb <= std::prev(it)->second) {
      // lb 以下から始まる区間を確認する
      if (ub <= std::prev(it)->second) return;  // 覆われていたら終了
 
      // 覆われていなければ lb を広げ，元の区間を一旦消去
      lb = std::prev(it)->first;
      ranges.erase(std::prev(it));
    }
 
    while (it != ranges.end() && it->first <= ub) {
      // 被っている区間を消していく．
      // ここにいい感じの制約が無いと計算量こわれる
      if (ub < it->second) ub = it->second;  // よしなに
      it = ranges.erase(it);
    }
 
    ranges.emplace(lb, ub);
  }
 
  void erase(Tp lb, Tp ub) {
    if (ranges.empty()) return;
 
    auto it=ranges.upper_bound({lb, lb});
 
    if (it != ranges.begin() && lb <= std::prev(it)->second) {
      if (ub <= std::prev(it)->second) {
        // [ ...* [ ...+ ) ...* ) みたいになっている
        --it;
        Tp lb0=it->first;
        Tp ub0=it->second;
        ranges.erase(it);
        if (lb0 < lb) ranges.emplace(lb0, lb);
        if (ub < ub0) ranges.emplace(ub, ub0);
        return;
      }
 
      // [ ...+ )      [ ...+ )*
      //      [ ...+ ) <- ここを消す      
      Tp lb0=std::prev(it)->first;
      ranges.erase(std::prev(it));
      ranges.emplace(lb0, lb);
    }
 
    while (it != ranges.end() && it->first <= ub) {
      if (ub < it->second) {
        // 完全には覆っていない場合
        Tp ub0=it->second;
        ranges.erase(it);
        ranges.emplace(ub, ub0);
        break;
      }
      it = ranges.erase(it);
    }
  }
 
  size_t num_ranges() const {
    return ranges.size();
  }
 
  std::pair<Tp, Tp> suprange(Tp x) const {
    // x を覆う区間があればそれを返します．なければ空を表す (x, x) を返します
    if (ranges.empty()) return {x, x};
 
    auto it=ranges.upper_bound({x, x});
 
    if (it != ranges.end())
      if (it->first <= x && x < it->second) return *it;
 
    if (it == ranges.begin() || (--it)->second <= x) return {x, x};
    return *it;
  }
 
  bool contains(Tp x) const {
    // 空でないとき，終端が x になることはありません
    return (suprange(x).second != x);
  }
 
  Tp max() const {
    if (ranges.empty()) throw std::logic_error("max in empty set");
    auto it=ranges.end();
    return (--it)->second - 1;
  }
 
  void inspect() const {
    for (auto it=ranges.begin(); it!=ranges.end(); ++it)
      std::cerr << '[' << it->first << ", " << it->second << ')'
                << (std::next(it)!=ranges.end()? ' ':'\n');
  }
};
