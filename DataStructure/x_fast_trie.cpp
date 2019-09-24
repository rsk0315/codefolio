template <size_t Nb, template <typename, typename> typename Map>
class x_fast_trie {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = uintmax_t;  // assuming transdichotomous machine model

private:
  static constexpr size_type S_bits = Nb;
  static constexpr size_type S_word_size = CHAR_BIT * sizeof(value_type);

  struct node {
    std::shared_ptr<node> parent = nullptr;  // for checking link type
    std::array<std::shared_ptr<node>, 2> children = {{nullptr, nullptr}};
    value_type value;
    node() = default;
    node(node const&) = default;  // or deep copy?
    node(node&&) = default;
  };

  using pointer = std::shared_ptr<node>;
  using map_type = Map<value_type, pointer>;
  std::array<map_type, S_bits> M_layers;

  pointer M_root = nullptr;
  size_type M_size = 0;

  void M_deep_copy(x_fast_trie const& other) {
    S_deep_copy(M_root, other.M_root);
  }

  static void S_deep_copy(pointer& dst, pointer const& src) {
    dst->value = src->value;
    for (int i = 0; i <= 1; ++i) {
      if (!src->children[i]) continue;
      if (src->children[i]->parent != src) continue;
      dst->children[i] = std::make_shared<node>();
      S_deep_copy(dst->children[i], src->children[i]);
    }
  }

  static value_type S_significant_n_bits(value_type x, size_type n) {
    n += S_word_size - S_bits;
    if (n >= S_word_size) return x;
    // fprintf(stderr, "(%ju & %jx): %jx\n",
    //         x, ~(~0_ju >> n), (x & ~(~0_ju >> n)));
    return x & ~(~0_ju >> n);
  }
  static value_type S_significant_nth_bit(value_type x, size_type n) {
    n += S_word_size - S_bits;
    if (n >= S_word_size) return 0;
    return x >> (S_word_size-1-n) & 1;
  }

  std::pair<size_type, pointer> M_lcp(value_type x) const {
    size_type lb = 0;
    size_type ub = S_bits;
    while (ub-lb > 1) {
      size_type mid = (lb+ub) >> 1;
      value_type x0 = S_significant_n_bits(x, mid+1);
      auto it = M_layers[mid].find(x0);
      ((it != M_layers[mid].end())? lb: ub) = mid;
    }
    // {length, pointer}
    auto it = M_layers[lb].find(S_significant_n_bits(x, ub));
    if (it == M_layers[lb].end()) return {0, M_root};
    return {ub, it->second};
  }

  static std::pair<pointer, pointer> S_neighbor(pointer const p) {
    fprintf(stderr, "neighbor of %p\n", p.get());
    pointer pre = nullptr;
    pointer suc = nullptr;
    fprintf(stderr, "left: %p, right: %p\n",
            p->children[0].get(), p->children[1].get());
    // if (p->children[0] && p->children[0]->parent != p) {
    if (S_is_thread(p, 0)) {
      suc = p->children[0];
      pre = suc->children[0];
    }
    // if (p->children[1] && p->children[1]->parent != p) {
    if (S_is_thread(p, 1)) {
      pre = p->children[1];
      suc = pre->children[1];
    }
    return {pre, suc};
  }

  static bool S_is_thread(pointer node, size_type dir) {
    if (!node || !node->children[dir]) return false;
    if (node->children[dir]->parent != node) return true;
    if (dir == 0) {
      return (node->children[dir]->value != node->value);
    } else {
      return (node->children[dir]->value == node->value);
    }
  }

  static void S_inspect(pointer root, int depth) {
    if (!root) return;

    // if (root->children[1] && root->children[1]->parent == root)
    if (!S_is_thread(root, 1))
      S_inspect(root->children[1], depth+1);

    fprintf(stderr, "%*c%p (%ju)", depth+1, ' ', root.get(), root->value);
    // fprintf(stderr, " (left: %p)", root->children[0].get());
    if (S_is_thread(root, 0))
      fprintf(stderr, " [pre: %ju]", root->children[0]->value);
    // fprintf(stderr, " (right: %p)", root->children[1].get());
    if (S_is_thread(root, 1))
      fprintf(stderr, " [suc: %ju]", root->children[1]->value);

    // fprintf(stderr, " (parent: %p)", root->parent.get());
    fprintf(stderr, "\n");

    if (!S_is_thread(root, 0))
      S_inspect(root->children[0], depth+1);
  }

public:
  x_fast_trie(): M_root(std::make_shared<node>()) {}
  x_fast_trie(x_fast_trie const& other) { M_deep_copy(other); }
  x_fast_trie(x_fast_trie&&) = default;

  size_type size() const { return M_size; }
  bool empty() const { return M_size == 0; }

  bool insert(value_type x) {
    size_type k;
    pointer p;
    std::tie(k, p) = M_lcp(x);
    if (k == S_bits) return false;
    pointer pre, suc;
    std::tie(pre, suc) = S_neighbor(p);

    while (k < S_bits) {
      // add nodes to trie
      // fprintf(stderr, "k: %zu\n", k);
      pointer tmp = std::make_shared<node>();
      p->children[S_significant_nth_bit(x, k++)] = tmp;
      M_layers[k-1][S_significant_n_bits(x, k)] = tmp;
      tmp->value = S_significant_n_bits(x, k);
      // fprintf(stderr, "<- %ju\n", S_significant_n_bits(x, k));
      tmp->parent = p;
      p = tmp;
    }
    {
      // maintain thread links
      pointer cur = p;
      fprintf(stderr, "pre: %p, suc: %p\n", pre.get(), suc.get());

      {
        cur->children[0] = pre;
        cur->children[1] = suc;
        if (pre) pre->children[1] = cur;
        if (suc) suc->children[0] = cur;
      }
      for (pointer p = cur->parent; p; p = p->parent) {
        if (!p->children[0] || (p->children[0]->parent != p && p->children[0]->value > x)) {
          p->children[0] = cur;
        }
        if (!p->children[1] || (p->children[1]->parent != p && p->children[1]->value < x)) {
          p->children[1] = cur;
        }
      }
    }
    ++M_size;
    return true;
  }
  bool erase(value_type x);
  size_type count(value_type x) const { return M_layers.back().count(x); }

  // value_type successor(value_type x) const { return *M_neighbor(x)[1]; }
  // value_type predecessor(value_type x) const { return *M_neighbor(x)[0]; }

  void inspect() const {
    S_inspect(M_root, 0);
    fprintf(stderr, "---\n");
  }
};
