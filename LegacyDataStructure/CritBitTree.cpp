template <class Key=std::string>
class CritBitTree {
  using CharT = typename Key::value_type;
  using MaskT = typename std::make_unsigned<CharT>::type;

  struct Node {
    size_t nth_byte=0;
    MaskT mask=0;
    Key *key;
    Node *children[2]={nullptr, nullptr};
    Node() = default;
    Node(const std::string &key): key(new std::string(key)) {}
  } *root=nullptr;

  Node *trace(const Key &key) {
    Node *pos=root;
    while (pos->children[0]) {
      MaskT byte=0;
      if (pos->nth_byte < key.length())
        byte = key[pos->nth_byte];

      size_t dir=!!(pos->mask & byte);
      pos = pos->children[dir];
    }
    return pos;
  }

  Node *trace(const Key &key, Node **subroot, size_t brdir) {
    Node *pos=root;
    while (pos->children[0]) {
      MaskT byte=0;
      if (pos->nth_byte < key.length())
        byte = key[pos->nth_byte];

      size_t dir=!!(pos->mask & byte);
      if (dir == brdir) *subroot = pos->children[!dir];
      pos = pos->children[dir];
    }
    return pos;
  }

  bool differs(
      const Key &key1, const Key &key2, size_t &nth_byte, MaskT &mask) {

    for (nth_byte=0; nth_byte<key1.length(); ++nth_byte)
      if ((mask = (key1[nth_byte] ^ key2[nth_byte])))
        return true;

    return (mask = key2[nth_byte]);
  }

  Node *find(const Key &key) {
    if (!root) return nullptr;

    Node *bestmatch=trace(key);
    size_t nth_byte;
    MaskT mask;
    if (!differs(key, *bestmatch->key, nth_byte, mask))
      return bestmatch;

    return nullptr;
  }

  const Node *find(const Key &key) const;

  template <class F>
  int traverse(Node *subroot, const F &callback) {
    fprintf(stderr, "subroot: [%zu:%02X]\n",
            subroot->nth_byte, subroot->mask);

    if (!subroot->children[0])
      return callback(*subroot->key);

    int ret=0;
    for (Node *subtree: subroot->children)
      if ((ret = traverse(subtree, callback)))
        break;

    return ret;
  }

  Node *neighbor(const Key &key, size_t brdir) {
    Node *subroot=nullptr;
    Node *bestmatch=trace(key, &subroot, brdir);

    size_t nth_byte;
    MaskT mask;
    if (differs(key, *bestmatch->key, brdir)) {
      for (MaskT tmp=mask; (tmp&=tmp-1); mask=tmp) {}

      Node *pos=root;
      subroot = nullptr;
      while (true) {
        if (!pos->children[0]) break;
        if (pos->nth_byte > nth_byte) break;
        if (pos->nth_byte == nth_byte && pos->mask < mask) break;

        MaskT byte=0;
        if (pos->nth_byte < key.length())
          byte = key[pos->nth_byte];

        size_t dir=!!(pos->mask & byte);
        if (dir == brdir) subroot = pos->children[brdir];
        pos = pos->children[dir];
      }

      size_t dir=!!(mask & (*bestmatch->key)[nth_byte]);
      if (dir != brdir) subroot = pos;
    }

    if (!subroot) return nullptr;
    Node *nleaf=subroot;
    while (nleaf->children[brdir])
      nleaf = nleaf->children[brdir];

    return nleaf;
  }

public:
  CritBitTree() = default;

  bool contains(const Key &key) {
    return find(key);
  }

  std::pair<Node *, bool> insert(const Key &key) {
    if (!root) {
      root = new Node(key);
      return {root, true};
    }

    Node *bestmatch=trace(key);
    size_t nth_byte;
    MaskT mask;
    if (!differs(key, *bestmatch->key, nth_byte, mask))
      return {bestmatch, false};

    for (MaskT tmp=mask; (tmp&=tmp-1); mask=tmp) {}

    size_t dir=!!(mask & (*bestmatch->key)[nth_byte]);
    Node *newnode=new Node;
    newnode->nth_byte = nth_byte;
    newnode->mask = mask;
    newnode->children[!dir] = new Node(key);

    Node **child=&root;
    while (true) {
      Node *pos=*child;
      if (!pos->children[0]) break;
      if (pos->nth_byte > nth_byte) break;
      if (pos->nth_byte == nth_byte && pos->mask < mask) break;

      MaskT byte=0;
      if (pos->nth_byte < key.length())
        byte = key[pos->nth_byte];

      size_t dir=!!(pos->mask & byte);
      child = &(pos->children[dir]);
    }
    newnode->children[dir] = *child;
    *child = newnode;

    return {newnode->children[!dir], true};
  }

  bool erase(const Key &key) {
    if (!root) return false;

    Node **pos=&root, **par=nullptr;
    size_t dir;
    while ((*pos)->children[0]) {
      const size_t nth_byte=(*pos)->nth_byte;
      MaskT byte=0;
      if (nth_byte < key.length())
        byte = key[nth_byte];

      dir = !!((*pos)->mask & byte);
      par = pos;
      pos = &((*pos)->children[dir]);
    }

    if (key != *(*pos)->key)
      return false;

    delete (*pos)->key;
    delete *pos;
    if (!par) {
      root = nullptr;
      return true;
    }

    // reduce the unneeded node, which does not diverge -- not a critical bit
    delete *par;
    *par = (*par)->children[!dir];
    return true;
  }

  template <class F>
  int fetch(const F &callback, const Key &prefix="") {
    if (!root) return -1;

    Node *pos=root, *subroot=pos;
    while (pos->children[0]) {
      MaskT byte=0;
      if (pos->nth_byte < prefix.length())
        byte = prefix[pos->nth_byte];

      size_t dir=!!(pos->mask & byte);
      if (pos->nth_byte < prefix.length())
        subroot = pos->children[dir];
      pos = pos->children[dir];
    }

    if (pos->key->compare(0, prefix.length(), prefix))
      return -1;

    return traverse(subroot, callback);
  }

  Node *next(const Key &key) {
    if (!root) return nullptr;
    if (!root->children[0]) {
      if (*root->key > key) return root->value;
      return nullptr;
    }
    return neighbor(key, 0);
  }

  Node *prev(const Key &key) {
    if (!root) return nullptr;
    if (!root->children[0]) {
      if (*root->key < key) return root->value;
      return nullptr;
    }
    return neighbor(key, 1);
  }

  /************************ Debug functions ************************/
  void debug() const {
    std::function<void (Node *, int)> dfs=[&](Node *subroot, int depth) {
      if (!subroot) return;

      dfs(subroot->children[1], depth+1);

      fprintf(stderr, "%s", std::string(depth, ' ').c_str());
      fprintf(stderr, "[%zu:%02x]", subroot->nth_byte, subroot->mask);
      if (subroot->key)
        fprintf(stderr, " (%s)", subroot->key->c_str());
      fprintf(stderr, "\n");

      dfs(subroot->children[0], depth+1);
    };
    fprintf(stderr, "================================================\n");
    dfs(root, 0);
    fprintf(stderr, "================================================\n");
  }
};
