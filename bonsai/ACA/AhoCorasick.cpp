#include <cstdio>
#include <cstdint>
#include <vector>
#include <set>
#include <algorithm>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <map>

size_t id = 0;

template <class String>
class matching_automaton {
  using char_type = typename String::value_type;
  struct node {
    size_t index;
    size_t length = 0;
    std::shared_ptr<node> output_link = nullptr;
    std::shared_ptr<node> suffix_link = nullptr;
    std::map<char_type, std::shared_ptr<node>> links;
    bool accepting = false;

    node(): index(id++) {}

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
      while (!(next || cur == root)) cur = cur->suffix_link;
      if (next) cur = next;
      if (cur->accepting) fn(s.substr(i+1-cur->length, cur->length));
      std::shared_ptr<node> out(cur->output_link);
      while (out) {
        fn(s.substr(i+1-out->length, out->length));
        out = out->output_link;
      }
    }
  }

  void dfs(std::shared_ptr<node> nd) const {
    fprintf(stderr, "index: %zu, length: %zu\n", nd->index, nd->length);
    if (nd->suffix_link) fprintf(stderr, "suffix-link: %zu\n", nd->suffix_link->index);
    if (nd->output_link) fprintf(stderr, "output-link: %zu\n", nd->output_link->index);
    for (const auto& p: nd->links) {
      fprintf(stderr, "[%c]:\n", p.first);
      dfs(p.second);
    }
    fprintf(stderr, "end index: %zu\n", nd->length);
  }

public:
  template <class ForwardIt>
  matching_automaton(ForwardIt first, ForwardIt last): root(new node) {
    build_nodes(first, last);
    build_links();
    // inspect();
  }

  void match(const String& s) const {
    match_do(s, [](const String& t) { printf("%s\n", t.c_str()); });
  }

  void inspect() const {
    dfs(root);
  }
};

int main() {
  std::vector<std::string> a{"i", "in", "tin", "sting"};
  matching_automaton<std::string> ma(a.begin(), a.end());

  ma.match("sting");
}
