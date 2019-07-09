template <typename Tp, size_t B = 3>
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

public:
  class snapshot {
    friend fully_persistent_vector;
    using snapshot_ptr = std::shared_ptr<snapshot>;
    base_ptr M_root;
    snapshot(base_ptr root): M_root(root) {}

  public:
    snapshot() = default;
    snapshot(snapshot const&) = default;
    snapshot(snapshot&&) = default;
    snapshot& operator =(snapshot const&) = default;
    snapshot& operator =(snapshot&&) = default;

    value_type get(size_type i) const {
      base_ptr node = M_root;
      size_t j = 0;
      size_t h = 0;
      while (j < i) {
        ++h;
        ++j <<= S_log_branch;
      }
      size_t shift = (h-1) * S_log_branch;
      while (h--) {
        j >>= S_log_branch;
        size_t b = (i-j) >> shift;
        node = node->children[b];
        i -= (b+1) << shift;
        shift -= S_log_branch;
      }
      return node->value;
    }

    snapshot set(size_type i, value_type const& x) {
      base_ptr root0(M_root);
      base_ptr root1(new M_node(root0->value));
      snapshot_ptr new_ss(new snapshot(root1));
      size_t j = 0;
      size_t h = 0;
      while (j < i) {
        ++h;
        ++j <<= S_log_branch;
      }
      size_t shift = (h-1) * S_log_branch;
      while (h--) {
        j >>= S_log_branch;
        size_type b = (i-j) >> shift;
        root1->children = root0->children;
        root0 = root0->children[b];
        root1 = root1->children[b] = base_ptr(new M_node(root0->value));
        i -= (b+1) << shift;
        shift -= S_log_branch;
      }
      root1->value = x;
      root1->children = root0->children;

      return *new_ss;
    }
  };
  using snapshot_ptr = std::shared_ptr<snapshot>;

private:
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
      v->children = {};
      for (size_type i = 0; i < S_branch; ++i) {
        v->children[i] = base_ptr(new M_node(x));
        q.push(v->children[i]);
        if (++m >= n) break;
      }
    }
    M_root = snapshot_ptr(new snapshot(root));
  }

  snapshot get_snapshot() const { return *M_root; }
};
