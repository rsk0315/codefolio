template <typename String>
class matching_automaton {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using char_type = typename String::value_type;
  using string_type = String;

private:
  struct M_node {
    using base_ptr = std::shared_ptr<M_node>;
    size_type length = 0;
    base_ptr output_link = nullptr;
    base_ptr suffix_link = nullptr;
    std::map<char_type, base_ptr> links;
    bool accepting = false;

    base_ptr append(char_type c) {
      auto it = links.lower_bound(c);
      if (it != links.end() && it->first == c) return it->second;
      base_ptr newnode(new M_node);
      newnode->length = length+1;
      links.emplace_hint(it, c, newnode);
      return newnode;
    }

    base_ptr find(char_type c) const {
      auto it = links.find(c);
      return (it != links.end())? it->second: nullptr;
    }
  };
  using base_ptr = std::shared_ptr<M_node>;
  base_ptr M_root;

  template <class InputIt>
  void M_build_nodes(InputIt first, InputIt last) {
    for (auto it = first; it != last; ++it) {
      base_ptr cur = M_root;
      for (char_type c: *it) cur = cur->append(c);
      cur->accepting = true;
    }
  }

  void M_build_links() {
    std::queue<base_ptr> q;
    for (const auto& p: M_root->links) {
      q.emplace(p.second);
      p.second->suffix_link = M_root;
    }

    while (!q.empty()) {
      base_ptr cur(q.front());
      q.pop();
      std::set<char_type> cc;
      for (const auto& p: cur->links) {
        cc.insert(p.first);
        p.second->suffix_link = M_root;
        q.emplace(p.second);
      }
      for (auto sf = cur->suffix_link; sf; sf = sf->suffix_link) {
        for (auto it = cc.begin(); it != cc.end();) {
          char_type c = *it;
          base_ptr dst(sf->find(c));
          if (!dst) {
            ++it;
            continue;
          }
          cur->find(c)->suffix_link = dst;
          it = cc.erase(it);
        }
        if (cc.empty()) break;
      }

      if (cur->suffix_link->accepting) {
        cur->output_link = cur->suffix_link;
      } else if (cur->suffix_link->output_link) {
        cur->output_link = cur->suffix_link->output_link;
      }
    }
  }

  template <class Functor>
  void M_match_do(string_type const& s, Functor fn) const {
    if (!M_root) return;
    base_ptr cur(M_root);
    for (size_type i = 0; i < s.length(); ++i) {
      char_type c = s[i];
      base_ptr next(cur->find(c));
      if (next) {
        cur = next;
      } else {
        while (cur != M_root && !cur->find(c)) cur = cur->suffix_link;
        if (cur->find(c)) cur = cur->find(c);
      }
      if (cur->accepting) fn(i+1-cur->length, cur->length);
      base_ptr out(cur->output_link);
      while (out) {
        fn(i+1-out->length, out->length);
        out = out->output_link;
      }
    }
  }

public:
  matching_automaton() = default;
  matching_automaton(matching_automaton const&) = default;
  matching_automaton(matching_automaton&&) = default;

  template <class InputIt>
  matching_automaton(InputIt first, InputIt last): M_root(new M_node) {
    M_build_nodes(first, last);
    M_build_links();
  }

  matching_automaton& operator =(matching_automaton const&) = default;
  matching_automaton& operator =(matching_automaton&&) = default;

  void clear() { M_root = nullptr; }

  std::vector<std::pair<size_type, size_type>> find_all_matches(string_type const& s) const {
    std::vector<std::pair<size_type, size_type>> res;
    M_match_do(s, [&](size_type i, size_type j) { res.emplace_back(i, j); });
    return res;
  }

  std::vector<size_type> longest_matches(string_type const& s) const {
    std::vector<size_type> res(s.length());
    M_match_do(s, [&](size_type i, size_type j) { res[i] = std::max(res[i], j); });
    return res;
  }

  void print_all_matches(string_type const& s) const {
    M_match_do(s, [&](size_type i, size_type j) {
        printf("%s\n", s.substr(i, j).c_str());
    });
  }

  difference_type count_matches(string_type const& s) const {
    difference_type res = 0;
    M_match_do(s, [&](size_type, size_type) { ++res; });
    return res;
  }
};

template <typename String>
class online_matching_automaton {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using char_type = typename String::value_type;
  using string_type = String;

private:
  std::vector<std::vector<string_type>> M_str;
  std::vector<matching_automaton<string_type>> M_pma;

public:
  online_matching_automaton() = default;

  void insert(string_type const& s) {
    std::vector<std::string> cur{s};
    for (size_type i = 0; i <= M_str.size(); ++i) {
      if (i == M_str.size() || M_str[i].empty()) {
        if (i == M_str.size()) {
          M_str.emplace_back();
          M_pma.emplace_back();
        }
        M_pma[i] = matching_automaton<string_type>(cur.begin(), cur.end());
        M_str[i] = std::move(cur);
        break;
      } else {
        assert(M_str[i].size() == cur.size());
        cur.insert(cur.end(), M_str[i].begin(), M_str[i].end());
        M_str[i].clear();
        M_pma[i].clear();
      }
    }
  }

  difference_type count_matches(string_type const& s) const {
    difference_type res = 0;
    for (size_type i = 0; i < M_pma.size(); ++i) {
      res += M_pma[i].count_matches(s);
    }
    return res;
  }
};
