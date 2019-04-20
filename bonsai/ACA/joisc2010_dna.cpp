#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <queue>
#include <memory>
#include <map>

template <class String>
class matching_automaton {
  using char_type = typename String::value_type;
  struct node {
    size_t length = 0;
    std::shared_ptr<node> output_link = nullptr;
    std::shared_ptr<node> suffix_link = nullptr;
    std::map<char_type, std::shared_ptr<node>> links;
    bool accepting = false;

    std::shared_ptr<node> append(char_type c) {
      auto it = links.lower_bound(c);
      if (it != links.end() && it->first == c) return it->second;
      std::shared_ptr<node> newnode(new node);
      newnode->length = length+1;
      links.emplace_hint(it, c, newnode);
      return newnode;
    }

    std::shared_ptr<node> find(char_type c) const {
      auto it = links.find(c);
      return (it != links.end())? it->second : nullptr;
    }
  };
  std::shared_ptr<node> root;

  template <class ForwardIt>
  void build_nodes(ForwardIt first, ForwardIt last) {
    for (auto it = first; it != last; ++it) {
      std::shared_ptr<node> cur = root;
      for (char_type c: *it) cur = cur->append(c);
      cur->accepting = true;
    }
  }

  void build_links() {
    std::queue<std::shared_ptr<node>> q;
    for (const auto& p: root->links) {
      q.emplace(p.second);
      p.second->suffix_link = root;
    }

    while (!q.empty()) {
      std::shared_ptr<node> cur(q.front());
      q.pop();
      std::set<char_type> cc;
      for (const auto& p: cur->links) {
        cc.insert(p.first);
        p.second->suffix_link = root;
        q.emplace(p.second);
      }
      for (auto sf = cur->suffix_link; sf; sf = sf->suffix_link) {
        for (auto it = cc.begin(); it != cc.end();) {
          char_type c = *it;
          std::shared_ptr<node> dst(sf->find(c));
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
  void match_do(const String& s, Functor fn) const {
    std::shared_ptr<node> cur(root);
    for (size_t i = 0; i < s.length(); ++i) {
      char_type c = s[i];
      std::shared_ptr<node> next(cur->find(c));
      if (next) {
        cur = next;
      } else {
        while (cur != root && !cur->find(c)) cur = cur->suffix_link;
        if (cur->find(c)) cur = cur->find(c);
      }
      if (cur->accepting) fn(i+1-cur->length, cur->length);
      std::shared_ptr<node> out(cur->output_link);
      while (out) {
        fn(i+1-out->length, out->length);
        out = out->output_link;
      }
    }
  }

public:
  template <class ForwardIt>
  matching_automaton(ForwardIt first, ForwardIt last): root(new node) {
    build_nodes(first, last);
    build_links();
  }

  std::vector<std::pair<size_t, size_t>> find_all_matches(const String& s) {
    std::vector<std::pair<size_t, size_t>> res;
    match_do(s, [&](size_t i, size_t j) { res.emplace_back(i, j); });
    return res;
  }

  std::vector<size_t> longest_matches(const String& s) {
    std::vector<size_t> res(s.length());
    match_do(s, [&](size_t i, size_t j) { res[i] = std::max(res[i], j); });
    return res;
  }

  void print_all_matches(const String& s) {
    match_do(s, [&](size_t i, size_t j) {
        printf("%s\n", s.substr(i, j).c_str());
    });
  }
};

int main() {
  size_t n;
  scanf("%zu", &n);

  char buf[262144];
  scanf("%s", buf);
  std::string s = buf;

  std::vector<std::string> pat(n);
  for (auto& pi: pat) {
    char buf[32];
    scanf("%s", buf);
    pi = buf;
  }

  matching_automaton<std::string> ma(pat.begin(), pat.end());

  auto ms = ma.longest_matches(s);
  size_t m = s.length();
  std::vector<size_t> dp(m+1, m+m);
  dp[0] = 0;
  for (size_t i = 0; i < m; ++i)
    for (size_t j = 1; j < ms[i]; ++j)
      dp[i+j] = std::min(dp[i+j], dp[i]+1);

  printf("%zu\n", dp[m-1]);
}
