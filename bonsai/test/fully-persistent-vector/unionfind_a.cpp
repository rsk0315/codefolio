#include <cstdio>
#include <algorithm>
#include <utility>
#include <vector>
#include <memory>
#include <queue>

template <typename Tp, size_t D = 32>
class fully_persistent_vector {
public:
  using size_type = size_t;
  using value_type = Tp;

private:
  constexpr static size_type S_branch = D;
  struct M_node {
    using base_ptr = std::shared_ptr<M_node>;
    value_type value;
    std::vector<base_ptr> children;
    M_node() = default;
    M_node(M_node const&) = default;
    M_node(M_node&&) = default;
    M_node(value_type const& x): value(x) {}
  };
  using base_ptr = std::shared_ptr<M_node>;

  struct M_snapshot {
    using snapshot_ptr = std::shared_ptr<M_snapshot>;
    base_ptr root = nullptr;
    snapshot_ptr parent = nullptr;
    std::vector<snapshot_ptr> children;
    M_snapshot() = default;
    M_snapshot(M_snapshot const&) = default;  // ?
    M_snapshot(M_snapshot&&) = default;
    M_snapshot(base_ptr root): root(root) {}
  };
  using snapshot_ptr = std::shared_ptr<M_snapshot>;
  snapshot_ptr M_root = nullptr;

public:
  fully_persistent_vector(size_type n, value_type const& x = value_type{}) {
    std::queue<base_ptr> q;
    base_ptr root = nullptr;
    size_type m = 0;
    if (n > 0) {
      root = base_ptr(new M_node(x));
      q.push(root);
      ++m;
    }
    while (!q.empty()) {
      if (m >= n) break;
      base_ptr v = q.front();
      q.pop();
      v->children.resize(S_branch, nullptr);
      for (size_type i = 0; i < S_branch; ++i) {
        v->children[i] = base_ptr(new M_node(x));
        q.push(v->children[i]);
        if (++m >= n) break;
      }
    }
    M_root = snapshot_ptr(new M_snapshot(root));
  }

  class snapshot {
    snapshot_ptr M_ss;

    static std::vector<size_type> S_way(size_type i) {
      std::vector<size_type> res;
      ++i;
      // fprintf(stderr, "[%zu]:", i);
      while (i > 1) {
        size_type j = i % S_branch;
        res.push_back((j+S_branch-2) % S_branch);
        i /= S_branch;
        if (j > 1) ++i;
      }
      std::reverse(res.begin(), res.end());
      // for (auto j: res) fprintf(stderr, " %zu", j);
      // fprintf(stderr, "\n");
      return res;
    }

  public:
    snapshot(snapshot_ptr ss): M_ss(ss) {}

    value_type get(size_type i) const {
      base_ptr node = M_ss->root;
      for (auto b: S_way(i)) node = node->children[b];
      return node->value;
    }

    snapshot set(size_type i, value_type const& x) {
      base_ptr root0(M_ss->root);
      base_ptr root1(new M_node(root0->value));
      snapshot_ptr new_ss(new M_snapshot(root1));
      auto way = S_way(i);
      for (size_type j = 0; j < way.size(); ++j) {
        size_type b = way[j];
        root1->children = root0->children;
        root0 = root0->children[b];
        root1 = root1->children[b] = base_ptr(new M_node(root0->value));
      }
      root1->value = x;
      root1->children = root0->children;

      M_ss->children.push_back(new_ss);
      new_ss->parent = M_ss;
      return snapshot(new_ss);
    }
  };

  snapshot get_snapshot() const {
    return snapshot(M_root);
  }
};

using snapshot = typename fully_persistent_vector<intmax_t>::snapshot;

size_t find(snapshot const& s, size_t v) {
  intmax_t p = s.get(v);
  if (p < 0) return v;
  return find(s, p);
}

snapshot unite(snapshot& s, size_t u, size_t v) {
  u = find(s, u);
  v = find(s, v);
  if (u == v) return s;  // s.set(0, s.get(0));
  intmax_t cu = s.get(u);
  intmax_t cv = s.get(v);
  if (-cu > -cv) {
    std::swap(u, v);
    std::swap(cu, cv);
  }
  snapshot s1 = s;
  s1 = s1.set(v, cv + cu);
  s1 = s1.set(u, v);
  return s1;
}

bool connected(snapshot const& s, size_t u, size_t v) {
  return find(s, u) == find(s, v);
}

int main() {
  size_t n, q;
  scanf("%zu %zu", &n, &q);
  fully_persistent_vector<intmax_t> fpa(n, -1);

  std::vector<snapshot> ss{fpa.get_snapshot()};
  for (size_t i = 0; i < q; ++i) {
    int t;
    size_t a, b;
    scanf("%d %zu %zu", &t, &a, &b);
    // --a, --b;
    snapshot s0 = ss.back();
    if (t == 0) {
      snapshot s1 = unite(s0, a, b);
      ss.push_back(s1);
    } else if (t == 1) {
      puts(connected(s0, a, b)? "Yes": "No");
    }
  }
}
