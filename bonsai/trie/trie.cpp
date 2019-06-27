#include <cstdio>
#include <cstdint>
#include <memory>
#include <map>

class trie {
public:
  using size_type = size_t;
  using char_type = char;

private:
  struct M_node {
    using base_ptr = std::shared_ptr<M_node>;
    std::map<char_type, base_ptr> children;
    base_ptr parent = nullptr;
    size_type self = 0;
    size_type subtree = 0;
  };

  using base_ptr = std::shared_ptr<M_node>;
  base_ptr M_root;

  size_type M_lcp(std::string const& s, base_ptr& node) const {
    size_type res = 0;
    node = M_root;
    while (res < s.length()) {
      auto it = node->children.find(s[res]);
      if (it == node->children.end()) break;
      ++res;
      node = it->second;
    }
    return res;
  }

  static void S_inspect_dfs(base_ptr node, std::string& cur) {
    fprintf(stderr, "node %p\n", node.get());
    if (node->self)
      fprintf(stderr, "contains %s (%zu)\n", cur.c_str(), node->self);
    for (auto const& p: node->children) {
      fprintf(stderr, "  [%c (0x%02x)]: %p\n", p.first, p.first, p.second.get());
      cur += p.first;
      S_inspect_dfs(p.second, cur);
      cur.pop_back();
    }
  }

public:
  trie(): M_root(new M_node) {}
  // trie(trie const& other): M_deep_copy(other) {}
  trie(trie&&) = default;
  // trie& operator =(trie const& other) { M_deep_copy(other); }
  trie& operator =(trie&&) = default;

  bool insert(std::string const& s) {
    // returns true if s is newly added the multiset
    base_ptr node;
    size_type pos = M_lcp(s, node);
    bool res = (pos < s.length());
    while (pos < s.length()) {
      base_ptr newnode(new M_node);
      node->children[s[pos++]] = newnode;
      node = newnode;
    }
    ++node->self;
    while (node) {
      ++node->subtree;
      node = node->parent;
    }
    return res;
  }

  bool erase(std::string const& s) {
    // returns true if s was in the multiset
    base_ptr node;
    size_type pos = M_lcp(s, node);
    if (pos != s.length()) return false;
    bool deleting = (--node->self == 0);
    while (node) {
      if (--node->subtree != 0) deleting = false;
      node = node->parent;
      if (node && deleting) node->children.at(s[--pos]) = nullptr;
    }
    return true;
  }

  size_type count(std::string const& s) const {
    base_ptr node;
    size_type pos = M_lcp(s, node);
    if (pos != s.length()) return 0;
    return node->self;
  }

  void inspect() const {
    std::string s;
    S_inspect_dfs(M_root, s);
  }
};

int main() {
  trie tr;
  tr.insert("abc");
  tr.insert("ax");
  tr.insert("a");
  printf("%zu\n", tr.count("a"));
  printf("%zu\n", tr.count("ab"));
  tr.inspect();
}
