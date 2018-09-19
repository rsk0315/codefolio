template <class Tp, size_t N=1>
class PersistentArray {
  static constexpr size_t branch=size_t(1)<<N;
  size_t size_=0;

  struct Node {
    const Tp *ptr;
    Node *children[branch]={};
    Node(const Tp &value=Tp()): ptr(new Tp(value)) {}
    Node(const Tp *ptr): ptr(ptr) {}
    const Tp &operator *() const { return *ptr; }
  } *root=nullptr;

  struct History {
    Node *root=nullptr;
    History *parent=nullptr;
    std::vector<History *> children={};
  } *history=nullptr;

  std::vector<size_t> navigate(size_t n) const {
    if (n == 0) return {};

    size_t width=branch, lvl=1;
    --n;
    while (n >= width) {
      n -= width;
      width *= branch;  // FIXME: optimize with considering base-2
      ++lvl;
    }
    std::vector<size_t> dir;
    for (size_t i=0; i<lvl; ++i) {
      dir.push_back(n % branch);
      n /= branch;
    }
    return dir;
  }

  const Node *nth_node(size_t n) const {
    if (n == 0) return root;

    std::vector<size_t> dir=navigate(n);
    Node *cur=root;
    for (size_t i=dir.size(); i--;)
      cur = cur->children[dir[i]];

    return cur;
  }

public:
  PersistentArray() {}

  PersistentArray(size_t n, const Tp &value=Tp()): size_(n) {
    if (n == 0) return;

    root = new Node(value);
    history = new History;
    history->root = root;
    --n;

    std::queue<Node *> q;
    q.emplace(root);
    while (true) {
      Node *cur=q.front();
      q.pop();
      for (size_t i=0; i<branch; ++i) {
        if (n-- == 0) return;
        cur->children[i] = new Node(value);  // this may be reduced
        q.emplace(cur->children[i]);
      }
    }
  }

  template <class ForwardIt>
  PersistentArray(ForwardIt first, ForwardIt last) {
    if (first == last) return;

    root = new Node(*first);
    size_ = 1;
    history = new History;
    history->root = root;
    ++first;

    std::queue<Node *> q;
    q.emplace(root);
    while (true) {
      Node *cur=q.front();
      q.pop();
      for (size_t i=0; i<branch; ++i) {
        if (first == last) return;
        cur->children[i] = new Node(*first++);
        ++size_;  // Note: we take ForwardIt
        q.emplace(cur->children[i]);
      }
    }
  }

  const Tp &operator [](size_t n) const {
    return **nth_node(n);
  }

  size_t size() const {
    return size_;
  }

  void update(size_t n, const Tp &value) {
    history->children.push_back(new History);
    history->children.back()->parent = history;
    history = history->children.back();

    if (n == 0) {
      Node *newroot=new Node(value);
      history->root = newroot;
      for (size_t i=0; i<branch; ++i)
        newroot->children[i] = root->children[i];

      root = newroot;
      return;
    }

    Node *newroot=new Node(root->ptr);
    history->root = newroot;
    std::vector<size_t> dir=navigate(n);
    Node *newcur=newroot, *cur=root;
    for (size_t i=dir.size(); i--;) {
      for (size_t j=0; j<branch; ++j) {
        if (j != dir[i]) {
          newcur->children[j] = cur->children[j];
        } else if (i > 0) {
          newcur->children[j] = new Node(cur->children[j]->ptr);
        } else {
          newcur->children[j] = new Node(value);
        }
      }
      newcur = newcur->children[dir[i]];
      cur = cur->children[dir[i]];
    }

    for (size_t j=0; j<branch; ++j)
      newcur->children[j] = cur->children[j];

    root = newroot;
  }

  void undo() {
    if (history->parent == nullptr)
      throw std::out_of_range("no further undo information");

    history = history->parent;
    root = history->root;
  }

  void inspect() const {
    std::cerr << '{';
    for (size_t i=0; i<size_; ++i)
      std::cerr << ' ' << (*this)[i];

    std::cerr << " }" << std::endl;
  }
};
