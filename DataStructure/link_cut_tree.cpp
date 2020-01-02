template <typename Tp>
class link_cut_tree {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = Tp;
  using reference = Tp&;
  using const_reference = Tp const&;

private:
  class node;
  using pointer = std::shared_ptr<node>;

  class node {
    friend link_cut_tree;

  public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using value_type = Tp;
    using reference = Tp&;
    using const_reference = Tp const&;

  private:
    using pointer = std::shared_ptr<node>;

    mutable pointer M_parent = nullptr;
    mutable std::array<pointer, 2> M_children{{nullptr, nullptr}};
    mutable size_type M_reversed = false;
    value_type M_value{};
    mutable std::array<value_type, 2> M_folded{{value_type{}, value_type{}}};

  public:
    node() = default;
    node(node const&) = delete;  // !
    node(node&&) = default;
    node(const_reference value): M_value(value), M_folded{{value, value}} {}
    node(value_type&& value):
      M_value(std::move(value)), M_folded{{M_value, M_value}} {}
  };

  static size_type S_parent_dir(pointer ptr) {
    if (!ptr->M_parent) return -1_zu;
    if (ptr->M_parent->M_children[0] == ptr) return 0;
    if (ptr->M_parent->M_children[1] == ptr) return 1;
    return -1_zu;
  }

  static pointer S_zig(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer sub = cur->M_children[!dir];

    cur->M_parent = par->M_parent;  // for parent path
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub;
    if (sub) sub->M_parent = par;

    S_resolve_folded(par);
    S_resolve_folded(cur);
    return cur;
  }
  static pointer S_zigzig(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[!dir];
    pointer sub2 = par->M_children[!dir];
    size_type gd = S_parent_dir(gpar);

    cur->M_parent = gpar->M_parent;
    if (gd != -1_zu) cur->M_parent->M_children[gd] = cur;
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    par->M_children[dir] = sub1;
    if (sub1) sub1->M_parent = par;
    par->M_children[!dir] = gpar;
    gpar->M_parent = par;
    gpar->M_children[dir] = sub2;
    if (sub2) sub2->M_parent = gpar;

    S_resolve_folded(gpar);
    S_resolve_folded(par);
    S_resolve_folded(cur);
    return cur;
  }
  static pointer S_zigzag(pointer cur, size_type dir) {
    pointer par = cur->M_parent;
    pointer gpar = par->M_parent;
    pointer sub1 = cur->M_children[dir];
    pointer sub2 = cur->M_children[!dir];
    size_type gd = S_parent_dir(gpar);

    cur->M_parent = gpar->M_parent;
    if (gd != -1_zu) cur->M_parent->M_children[gd] = cur;
    cur->M_children[dir] = gpar;
    gpar->M_parent = cur;
    cur->M_children[!dir] = par;
    par->M_parent = cur;
    gpar->M_children[!dir] = sub1;
    if (sub1) sub1->M_parent = gpar;
    par->M_children[dir] = sub2;
    if (sub2) sub2->M_parent = par;

    S_resolve_folded(gpar);
    S_resolve_folded(par);
    S_resolve_folded(cur);
    return cur;
  }

  static void S_resolve_folded(pointer cur) {
    for (size_type i = 0; i <= 1; ++i) {
      pointer& first = cur->M_children[i];
      pointer& second = cur->M_children[!i];
      if (first) {
        cur->M_folded[i] = first->M_folded[i ^ first->M_reversed];
      } else {
        cur->M_folded[i] = value_type{};
      }
      cur->M_folded[i] += cur->M_value;
      if (second)
        cur->M_folded[i] += second->M_folded[i ^ second->M_reversed];
    }
  }

  static void S_resolve_reverse(pointer cur) {
    if (!cur->M_reversed) return;
    cur->M_reversed = 0;
    for (size_type i = 0; i <= 1; ++i) {
      if (cur->M_children[i]) {
        std::swap(cur->M_children[i]->M_folded[0],
                  cur->M_children[i]->M_folded[1]);
        cur->M_children[i]->M_reversed ^= 1;
      }
    }
    std::swap(cur->M_children[0], cur->M_children[1]);
    S_resolve_folded(cur);
  }

  static pointer S_splay(pointer cur) {
    if (!cur) return nullptr;
    while (true) {
      size_type pd = S_parent_dir(cur);
      if (pd == -1_zu) return S_resolve_reverse(cur), cur;
      pointer p = cur->M_parent;
      size_type gd = S_parent_dir(p);
      if (gd == -1_zu) {
        if (p->M_reversed) S_resolve_reverse(p), pd ^= 1;
        if (cur->M_reversed) S_resolve_reverse(cur);
        return S_zig(cur, pd);
      }
      pointer g = p->M_parent;

      if (g->M_reversed) S_resolve_reverse(g), gd ^= 1;
      if (p->M_reversed) S_resolve_reverse(p), pd ^= 1;
      if (cur->M_reversed) S_resolve_reverse(cur);
      cur = ((gd == pd)? S_zigzig(cur, pd): S_zigzag(cur, pd));
    }
  }

private:
  static void S_expose(pointer src) {
    pointer right = nullptr;
    for (pointer cur = src; cur; cur = cur->M_parent) {
      S_splay(cur);
      assert(!cur->M_reversed);
      cur->M_children[1] = right;
      S_resolve_folded(cur);
      right = cur;
    }
    S_splay(src);
  }

  static void S_evert(pointer src) {
    S_expose(src);
    src->M_reversed ^= 1;
    S_resolve_reverse(src);
  }

public:
  link_cut_tree() = default;
  link_cut_tree(link_cut_tree const&) = delete;  // !
  link_cut_tree(link_cut_tree&&) = default;

  link_cut_tree& operator =(link_cut_tree const&) = delete;  // !
  link_cut_tree& operator =(link_cut_tree&&) = default;

  class node_handle {
    friend link_cut_tree;
    pointer M_ptr = nullptr;
    node_handle(pointer ptr): M_ptr(ptr) {}

  public:
    node_handle() = default;
    node_handle(node_handle const&) = default;
    node_handle(node_handle&&) = default;

    node_handle& operator =(node_handle const&) = default;
    node_handle& operator =(node_handle&&) = default;

    const_reference& operator *() const { return M_ptr->M_value; }
  };

  node_handle add_node() { return std::make_shared<node>(); }
  node_handle add_node(const_reference value) {
    return std::make_shared<node>(value);
  }
  node_handle add_node(value_type&& value) {
    return std::make_shared<node>(std::move(value));
  }

  void link(node_handle curh, node_handle parh) {
    pointer cur = curh.M_ptr;
    pointer par = parh.M_ptr;
    S_evert(cur);
    S_expose(par);
    cur->M_parent = par;
    par->M_children[1] = cur;
    S_resolve_folded(par);
  }

  void cut(node_handle curh) {
    pointer cur = curh.M_ptr;
    S_expose(cur);
    pointer par = cur->M_children[0];
    par->M_parent = nullptr;
    cur->M_children[0] = nullptr;
    S_resolve_folded(cur);
  }

  void evert(node_handle posh) const { S_evert(posh.M_ptr); }
  void expose(node_handle posh) const { S_expose(posh.M_ptr); }

  void update(node_handle posh, const_reference value) {
    evert(posh);
    pointer pos = posh.M_ptr;
    pos->M_value = value;
    S_resolve_folded(pos);
  }
  void update(node_handle posh, value_type&& value) {
    evert(posh);
    pointer pos = posh.M_ptr;
    pos->M_value = std::move(value);
    S_resolve_folded(pos);
  }

  value_type fold_path(node_handle srch, node_handle dsth) {
    pointer src = srch.M_ptr;
    pointer dst = dsth.M_ptr;
    S_evert(src);
    S_expose(dst);
    return dst->M_folded[0];
  }
};
