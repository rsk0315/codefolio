class EmdeBoasTree;

template <size_t Height>
class EmdeBoasTreeNode {
  friend EmdeBoasTree;
  friend EmdeBoasTreeNode<Height+1>;
  using IndexType = unsigned long long;
  static constexpr int HALF_WIDTH=1<<(Height-1);

  IndexType min_=1, max_=0;
  EmdeBoasTreeNode<Height-1> *summary=nullptr;
  std::map<IndexType, EmdeBoasTreeNode<Height-1>> cluster={};

  EmdeBoasTreeNode() = default;

  constexpr IndexType upper(IndexType u) const {
    return u >> HALF_WIDTH;
  }

  constexpr IndexType lower(IndexType u) const {
    return u & ((IndexType(1) << HALF_WIDTH) - 1);
  }

  bool contains(IndexType u) const {
    if (u == min_ || u == max_) return true;
    if (u < min_ || max_ < u) return false;
    if (summary == nullptr || !summary->contains(upper(u))) return false;
    assert(cluster.find(upper(u)) != cluster.end());
    return cluster.at(upper(u)).contains(lower(u));
  }

  IndexType successor(IndexType u) const {
    if (empty() || u >= max_) throw std::out_of_range("no successor");
    if (u < min_) return min_;
    if (summary == nullptr) return max_;

    IndexType uu=upper(u), lu=lower(u);
    if (summary->contains(uu) && lu < cluster.at(uu).max())
      return uu << HALF_WIDTH | cluster.at(uu).successor(lu);

    if (uu >= summary->max()) return max_;
    IndexType bucket=summary->successor(uu);
    return bucket << HALF_WIDTH | cluster.at(bucket).min();
  }

  IndexType predecessor(IndexType u) const {
    if (empty() || u <= min_) throw std::out_of_range("no predecessor");
    if (u > max_) return max_;
    if (summary == nullptr) return min_;

    IndexType uu=upper(u), lu=lower(u);
    if (summary->contains(uu) && lu > cluster.at(uu).min())
      return uu << HALF_WIDTH | cluster.at(uu).predecessor(lu);

    if (uu <= summary->min()) return min_;
    IndexType bucket=summary->predecessor(uu);
    return bucket << HALF_WIDTH | cluster.at(bucket).max();
  }

  bool insert(IndexType u) {
    if (empty()) {
      min_ = max_ = u;
      return true;
    }
    if (u == min_ || u == max_) return false;

    if (min_ == max_) {
      if (max_ < u) {
        max_ = u;
        return true;
      }
      min_ = u;
      return true;
    }

    if (min_ > u) {
      std::swap(min_, u);
    } else if (max_ < u) {
      std::swap(max_, u);
    }

    if (summary == nullptr) summary = new EmdeBoasTreeNode<Height-1>;
    summary->insert(upper(u));

    auto it=cluster.lower_bound(upper(u));
    if (it == cluster.end() || it->first != upper(u))
      it = cluster.emplace_hint(it, upper(u), EmdeBoasTreeNode<Height-1>());

    return it->second.insert(lower(u));
  }

  bool erase(IndexType u) {
    if (min_ == max_) {
      if (min_ == u) {
        min_ = 1;
        max_ = 0;
        return true;
      } else {
        return false;
      }
    }

    if (u < min_ || max_ < u) return false;

    if (summary == nullptr) {
      if (u == min_) {
        min_ = max_;
        return true;
      } else if (u == max_) {
        max_ = min_;
        return true;
      } else {
        return false;
      }
    }

    IndexType bucket=upper(u), v=lower(u);
    if (u == min_) {
      bucket = summary->min();
      v = cluster.at(bucket).min();
      min_ = bucket << HALF_WIDTH | v;
    } else if (u == max_) {
      bucket = summary->max();
      v = cluster.at(bucket).max();
      max_ = bucket << HALF_WIDTH | v;
    } else {
      if (!summary->contains(bucket)) return false;
    }
    bool res=cluster.at(bucket).erase(v);
    if (res && cluster.at(bucket).empty()) {
      cluster.erase(bucket);
      summary->erase(bucket);
      if (summary->empty()) {
        delete summary;
        summary = nullptr;
      }
    }
    return res;
  }

  IndexType min() const {
    if (empty()) throw std::out_of_range("min() on {}");
    return min_;
  }

  IndexType max() const {
    if (empty()) throw std::out_of_range("max() on {}");
    return max_;
  }

  bool empty() const {
    return max_ < min_;
  }
};

template <>
class EmdeBoasTreeNode<3> {
  friend EmdeBoasTree;
  friend EmdeBoasTreeNode<4>;
  using IndexType = unsigned long long;

  IndexType vec[4]={};

  EmdeBoasTreeNode() = default;

  bool contains(IndexType u) const {
    return vec[u/64] >> (u%64) & 1;
  }

  IndexType successor(IndexType u) const {
    IndexType tmp=vec[u/64]&-(1uLL<<((u+1)%64));
    if (tmp > 0) return (u/64*64) | __builtin_ctzll(tmp);
    for (size_t i=u/64+1; i<4; ++i)
      if (vec[i]) return i<<6 | __builtin_ctzll(vec[i]);

    throw std::out_of_range("u <= minimum");
  }

  IndexType predecessor(IndexType u) const {
    IndexType tmp=vec[u/64]&((1uLL<<(u%64))-1);
    if (tmp > 0) return (u/64*64) | (63-__builtin_clzll(tmp));
    for (size_t i=u/64; i--;)
      if (vec[i]) return i<<6 | (63-__builtin_clzll(vec[i]));

    throw std::out_of_range("u >= maximum");
  }

  bool insert(IndexType u) {
    if (vec[u/64] >> (u%64) & 1) return false;
    vec[u/64] |= 1uLL << (u%64);
    return true;
  }

  bool erase(IndexType u) {
    if (!(vec[u/64] >> (u%64) & 1)) return false;
    vec[u/64] ^= 1uLL << (u%64);
    return true;
  }

  IndexType min() const {
    for (int i=0; i<4; ++i)
      if (vec[i]) return i<<6 | __builtin_ctzll(vec[i]);

    throw std::out_of_range("min() on {}");
  }

  IndexType max() const {
    for (size_t i=4; i--;)
      if (vec[i]) return i<<6 | (63-__builtin_clzll(vec[i]));

    throw std::out_of_range("max() on {}");
  }

  bool empty() const {
    return !(vec[0] || vec[1] || vec[2] || vec[3]);
  }
};

class EmdeBoasTree {
  using IndexType = unsigned long long;
  static constexpr size_t TopHeight=5;

  EmdeBoasTreeNode<TopHeight> *root;
  size_t size_=0;

public:
  EmdeBoasTree(): root(new EmdeBoasTreeNode<TopHeight>) {}

  IndexType min() const { return root->min(); }
  IndexType max() const { return root->max(); }
  bool empty() const { return root->empty(); }
  bool contains(IndexType u) const { return !empty() && root->contains(u); }
  IndexType successor(IndexType u) const { return root->successor(u); }
  IndexType predecessor(IndexType u) const { return root->predecessor(u); }
  bool insert(IndexType u) { return root->insert(u) && ++size_; }
  bool erase(IndexType u) { return !empty() && root->erase(u) && size_--; }
  size_t size() const { return size_; }

  void debug() const {
    if (empty()) {
      fprintf(stderr, "{}\n");
      return ;
    }
    fprintf(stderr, "{");
    IndexType i_ub=max();
    for (IndexType i=min(); i<i_ub; i=successor(i)) {
      fprintf(stderr, "%llu%s", i, i<i_ub? ", ":"");
      assert(contains(successor(i)));
      assert(i < successor(i));
    }

    fprintf(stderr, "%llu}\n", i_ub);
    fflush(stderr);
  }
};
