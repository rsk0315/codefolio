class sequences {
  // sigma 種類の値からなる系列の集合に対するクエリ処理．
  // 最初の系列 s_0 は空列であり，以下を満たす s_i が作れるかを答える．
  // - s_i の長さは q_i である．
  // - s_i は辞書順で s_{i-1} より真に大きい
  // s_i としてありうる系列のうちで辞書順最小のものを RLE で持ってある．

  size_t sigma;
  size_t length = 0;
  std::vector<std::pair<size_t, size_t>> s;  // {alpha, run}

  void truncate_to(size_t next_len) {
    assert(!s.empty());
    size_t last = s.back().first;
    while (length > next_len) {
      size_t run;
      std::tie(last, run) = s.back();
      s.pop_back();
      length -= run;
    }
    if (length == next_len) return;
    s.emplace_back(last, next_len-length);
    length = next_len;
  }

  bool increment() {
    assert(!s.empty());
    size_t& alpha = s.back().first;
    size_t& run = s.back().second;
    if (run == 1) {
      ++alpha;
      size_t slen = s.size();
      if (slen >= 2 && s[slen-2].first == alpha) {
        s.pop_back();
        ++s.back().second;
      }
    } else {
      --run;
      s.emplace_back(alpha+1, 1);  // OK?
    }
    if (s.back().first < sigma) return true;
    return resolve_carry();
  }

  bool resolve_carry() {
    assert(s.back().first == sigma);
    assert(s.back().second == 1);
    
    s.pop_back();
    if (s.empty()) return false;
    size_t run = 1;
    if (s.back().first+1 == sigma) {
      run += s.back().second;
      s.pop_back();
    }
    if (s.empty()) return false;
    increment();
    s.emplace_back(0, run);
    return true;
  }

public:
  sequence(size_t sigma): sigma(sigma) {}

  bool next_sequence(size_t next_len) {
    if (sigma == 0) return false;
    if (length == 0) {
      s.emplace_back(0, next_len);
      length = next_len;
      return true;
    }
    if (length < next_len) {
      if (s.back().first == 0) {
        s.back().second += next_len-length;
      } else {
        s.emplace_back(0, next_len-length);
      }
      length = next_len;
      return true;
    }

    truncate_to(next_len);
    return increment();
  }

  void show() const {
    fprintf(stderr, "%zu:", length);
    for (const auto& p: s)
      for (size_t i = 0; i < p.second; ++i)
        fprintf(stderr, " %s%zu%s",
                ((i > 0)? "":"("), p.first,
                ((i+1 < p.second)? "":")"));

    fprintf(stderr, "\n");
  }
};
