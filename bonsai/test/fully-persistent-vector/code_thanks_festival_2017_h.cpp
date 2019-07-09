#include <cstdio>
#include <algorithm>
#include <utility>
#include <vector>
#include <memory>
#include <queue>
#include <array>

template <typename Tp, size_t B = 4>
class fully_persistent_vector {
public:
  using size_type = size_t;
  using value_type = Tp;

private:
  constexpr static size_type S_log_branch = B;
  constexpr static size_type S_branch = 1u << B;
  struct M_node {
    using base_ptr = std::shared_ptr<M_node>;
    value_type value;
    std::array<base_ptr, S_branch> children;
    M_node() = default;
    M_node(M_node const&) = default;
    M_node(M_node&&) = default;
    M_node(value_type const& x): value(x) {}
  };
  using base_ptr = std::shared_ptr<M_node>;

  struct M_snapshot {
    using snapshot_ptr = std::shared_ptr<M_snapshot>;
    base_ptr root = nullptr;
    // snapshot_ptr parent = nullptr;
    // std::vector<snapshot_ptr> children;
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
      // v->children.resize(S_branch, nullptr);
      v->children = {};
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

  public:
    snapshot(snapshot_ptr ss): M_ss(ss) {}

    value_type get(size_type i) const {
      base_ptr node = M_ss->root;
      size_t j = 0;
      size_t h = 0;
      while (j < i) {
        ++h;
        ++j <<= S_log_branch;
      }
      while (h--) {
        j >>= S_log_branch;
        size_t b = (i-j) >> (h * S_log_branch);
        node = node->children[b];
        i -= (b+1) << (h * S_log_branch);
      }
      return node->value;
    }

    snapshot set(size_type i, value_type const& x) {
      base_ptr root0(M_ss->root);
      base_ptr root1(new M_node(root0->value));
      snapshot_ptr new_ss(new M_snapshot(root1));
      size_t j = 0;
      size_t h = 0;
      while (j < i) {
        ++h;
        ++j <<= S_log_branch;
      }
      while (h--) {
        j >>= S_log_branch;
        size_type b = (i-j) >> (h * S_log_branch);
        root1->children = root0->children;
        root0 = root0->children[b];
        root1 = root1->children[b] = base_ptr(new M_node(root0->value));
        i -= (b+1) << (h * S_log_branch);
      }
      root1->value = x;
      root1->children = root0->children;

      // M_ss->children.push_back(new_ss);
      // new_ss->parent = M_ss;
      return snapshot(new_ss);
    }
  };

  snapshot get_snapshot() const {
    return snapshot(M_root);
  }
};

using snapshot = typename fully_persistent_vector<int>::snapshot;

size_t find(snapshot const& s, size_t v) {
  int p = s.get(v);
  while (p >= 0) {
    v = p;
    p = s.get(v);
  }
  return v;
}

snapshot unite(snapshot& s, size_t u, size_t v) {
  u = find(s, u);
  v = find(s, v);
  if (u == v) return s.set(0, s.get(0));
  int cu = s.get(u);
  int cv = s.get(v);
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
  size_t n, m;
  scanf("%zu %zu", &n, &m);
  fully_persistent_vector<int> fpa(n, -1);

  std::vector<snapshot> ss{fpa.get_snapshot()};
  ss.reserve(m+1);
  for (size_t i = 0; i < m; ++i) {
    size_t a, b;
    scanf("%zu %zu", &a, &b);
    --a, --b;
    snapshot s0 = ss.back();
    snapshot s1 = unite(s0, a, b);
    ss.push_back(s1);
  }

  size_t q;
  scanf("%zu", &q);
  for (size_t i = 0; i < q; ++i) {
    size_t x, y;
    scanf("%zu %zu", &x, &y);
    --x, --y;

    if (!connected(ss[m], x, y)) {
      puts("-1");
      continue;
    }

    size_t lb = 0;
    size_t ub = m;
    while (ub-lb > 1) {
      size_t mid = (lb+ub) >> 1;
      (!connected(ss[mid], x, y)? lb: ub) = mid;
    }
    printf("%zu\n", ub);
  }
}
