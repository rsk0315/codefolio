#ifndef STL_LIKE_H
#define STL_LIKE_H

#include <iterator>
#include <type_traits>
#include <memory>

#include <iostream>

namespace stl_like
{
  namespace detail
  {
    // Supporting structures are split into common and templated
    // types; the latter publicly inherits from the former in an
    // effort to reduce code duplication.  This results in some
    // "needless" static_cast'ing later on, but it's all safe
    // downcasting.

    /// Common part of a node in the %list.
    struct List_node_base
    {
      List_node_base* M_next;
      List_node_base* M_prev;

      static void
      swap(List_node_base& x, List_node_base& y) noexcept;

      void
      M_transfer(List_node_base* const first,
                 List_node_base* const last) noexcept;

      void
      M_reverse() noexcept;

      void
      M_hook(List_node_base* const position) noexcept;

      void
      M_unhook() noexcept;
    };

    /// The %list node header.
    struct List_node_header: public List_node_base
    {
      size_t M_size;

      List_node_header() noexcept
      { M_init(); }

      List_node_header(List_node_header&& x) noexcept
        : List_node_base{x.M_next, x.M_prev}
        , M_size(x.M_size)
      {
        if (x.M_base()->M_next == x.M_base()) {
          this->M_next = this->M_prev = this;
        }
        else {
          this->M_next->M_prev = this->M_prev->M_next = this->M_base();
          x.M_init();
        }
      }

      void
      M_move_nodes(List_node_header&& x)
      {
        List_node_base* const xnode = x.M_base();
        if (xnode->M_next == xnode) {
          M_init();
        }
        else {
          List_node_base* const node = this->M_base();
          node->M_next = xnode->M_next;
          node->M_prev = xnode->M_prev;
          node->M_next->M_prev = node->M_prev->M_next = node;
          M_size = x.M_size;
          x.M_init();
        }
      }

      void
      M_init() noexcept
      {
        this->M_next = this->M_prev = this;
        this->M_size = 0;
      }

    private:
      List_node_base* M_base()
      { return this; }
    };
  }  // namespace detail

  /// An actual node in the %list.
  template <typename Tp>
  struct List_node: public detail::List_node_base
  {
    Tp M_data;
    Tp*       M_valptr()       { return std::addressof(M_data); }
    Tp const* M_valptr() const { return std::addressof(M_data); }
  };

  /**
   *  @ brief A list::iterator.
   *
   *  All the functions are op overloads.
   */
  template <typename Tp>
  struct List_iterator
  {
    typedef List_iterator<Tp>                   Self;
    typedef List_node<Tp>                       Node;

    typedef ptrdiff_t                           difference_type;
    typedef std::bidirectional_iterator_tag     iterator_category;
    typedef Tp                                  value_type;
    typedef Tp*                                 pointer;
    typedef Tp&                                 reference;

    List_iterator() noexcept
      : M_node() {}

    explicit
    List_iterator(detail::List_node_base* x) noexcept
      : M_node(x) {}

    Self
    M_const_cast() const noexcept
    { return *this; }

    // Must downcast from List_node_base to List_node to get to value
    reference
    operator *() const noexcept
    { return *static_cast<Node*>(M_node)->M_valptr(); }

    pointer
    operator ->() const noexcept
    { return static_cast<Node*>(M_node)->M_valptr(); }

    Self&
    operator ++() noexcept
    {
      M_node = M_node->M_next;
      return *this;
    }

    Self
    operator ++(int) noexcept
    {
      Self tmp = *this;
      M_node = M_node->M_next;
      return tmp;
    }

    Self&
    operator --() noexcept
    {
      M_node = M_node->M_prev;
      return *this;
    }

    Self
    operator --(int) noexcept
    {
      Self tmp = *this;
      M_node = M_node->M_prev;
      return tmp;
    }

    bool
    operator ==(const Self& x) const noexcept
    { return M_node == x.M_node; }

    bool
    operator !=(const Self& x) const noexcept
    { return M_node != x.M_node; }

    // The only member points to the %list element
    detail::List_node_base* M_node;
  };

  /**
   *  @brief A list::const_iterator.
   *
   *  All the functions are op overloads.
   */
  template <typename Tp>
  struct List_const_iterator
  {
    typedef List_const_iterator<Tp>             Self;
    typedef const List_node<Tp>                 Node;
    typedef List_iterator<Tp>                   iterator;

    typedef ptrdiff_t                           difference_type;
    typedef std::bidirectional_iterator_tag     iterator_category;
    typedef Tp                                  value_type;
    typedef const Tp*                           pointer;
    typedef const Tp&                           reference;

    List_const_iterator() noexcept
      : M_node() {}

    explicit
    List_const_iterator(const detail::List_node_base* x) noexcept
      : M_node(x) {}

    List_const_iterator(const iterator& x) noexcept
      : M_node(x.M_node) {}

    iterator
    M_const_cast() const noexcept
    { return iterator(const_cast<detail::List_node_base*>(M_node)); }

    // Must downcast from List_node_base to List_node to get to value.
    reference
    operator *() const noexcept
    { return *static_cast<Node*>(M_node)->M_valptr(); }

    pointer
    operator ->() const noexcept
    { return static_cast<Node*>(M_node)->M_valptr(); }

    Self&
    operator ++() noexcept
    {
      M_node = M_node->M_next;
      return *this;
    }

    Self
    operator ++(int) noexcept
    {
      Self tmp = *this;
      M_node = M_node->M_next;
      return tmp;
    }

    Self&
    operator --() noexcept
    {
      M_node = M_node->M_prev;
      return *this;
    }

    Self
    operator --(int) noexcept
    {
      Self tmp = *this;
      M_node = M_node->M_prev;
      return tmp;
    }

    bool
    operator ==(const Self& x) const noexcept
    { return M_node == x.M_node; }

    bool
    operator !=(const Self& x) const noexcept
    { return M_node != x.M_node; }

    // The only member points to the %list element.
    const detail::List_node_base* M_node;
  };

  template <typename Val>
  inline bool
  operator ==(const List_iterator<Val>& x,
              const List_const_iterator<Val>& y) noexcept
  { return x.M_node == y.M_node; }

  template <typename Val>
  inline bool
  operator !=(const List_iterator<Val>& x,
              const List_const_iterator<Val>& y) noexcept
  { return x.M_node != y.M_node; }

  /**
   *  List base class.  This class provides the unified face for %list's
   *  allocation.  This class's constructor and destructor allocate and
   *  deallocate (but do not initialize) storage.  This makes %exception
   *  safety easier.
   *
   *  Nothing in this class ever constructs or destroys an actual Tp element.
   *  (List handles that itself.)  Only/All memory management is performed
   *  here
   */
  template <typename Tp, typename Alloc>
  class List_base
  {
  protected:
    typedef typename std::allocator_traits<Alloc>::template
        rebind_alloc<Tp>                                Tp_alloc_type;
    typedef std::allocator_traits<Tp_alloc_type>        Tp_alloc_traits;
    typedef typename Tp_alloc_traits::template
        rebind_alloc<List_node<Tp>>                     Node_alloc_type;
    typedef std::allocator_traits<Node_alloc_type>      Node_alloc_traits;

    static size_t
    S_distance(const detail::List_node_base* first,
               const detail::List_node_base* last)
    {
      size_t n = 0;
      while (first != last) {
        first = first->M_next;
        ++n;
      }
      return n;
    }

    struct List_impl: public Node_alloc_type
    {
      detail::List_node_header M_node;

      List_impl() noexcept(noexcept(Node_alloc_type()))
        : Node_alloc_type() {}

      List_impl(const Node_alloc_type& a) noexcept
        : Node_alloc_type(a) {}

      List_impl(List_impl&&) = default;

      List_impl(Node_alloc_type&& a, List_impl&& x)
        : Node_alloc_type(std::move(a)), M_node(std::move(x.M_node)) {}

      List_impl(Node_alloc_type&& a) noexcept
        : Node_alloc_type(std::move(a)) {}
    };

    List_impl M_impl;

    size_t M_get_size() const { return M_impl.M_node.M_size; }

    void M_set_size(size_t n) { M_impl.M_node.M_size = n; }
    void M_inc_size(size_t n) { M_impl.M_node.M_size += n; }
    void M_dec_size(size_t n) { M_impl.M_node.M_size -= n; }

    size_t
    M_distance(const detail::List_node_base* first,
               const detail::List_node_base* last) const
    { return S_distance(first, last); }

    // return the stored size
    size_t M_node_count() const { return M_get_size(); }

    typename Node_alloc_traits::pointer
    M_get_node()
    { return Node_alloc_traits::allocate(M_impl, 1); }

    void
    M_put_node(typename Node_alloc_traits::pointer p) noexcept
    { Node_alloc_traits::deallocate(M_impl, p, 1); }

  public:
    typedef Alloc allocator_type;

    Node_alloc_type&
    M_get_Node_allocator() noexcept
    { return M_impl; }

    const Node_alloc_type&
    M_get_Node_allocator() const noexcept
    { return M_impl; }

    List_base() = default;

    List_base(const Node_alloc_type& a) noexcept
      : M_impl(a) {}

    List_base(List_base&&) = default;

    List_base(List_base&& x, Node_alloc_type&& a)
      : M_impl(std::move(a))
    {
      if (x.M_get_Node_allocator() == M_get_Node_allocator())
        M_move_nodes(std::move(x));
      // else caller must move individual elements.
    }

    // Used when allocator is_always_equal
    List_base(Node_alloc_type&& a, List_base&& x)
      : M_impl(std::move(a), std::move(x.M_impl)) {}

    // Used when allocator !is_always_equal.
    List_base(Node_alloc_type&& a)
      : M_impl(std::move(a)) {}

    void
    M_move_nodes(List_base&& x)
    { M_impl.M_node.M_move_nodes(std::move(x.M_impl.M_node)); }

    // This is what actually destroys the list.
    ~List_base() noexcept
    { M_clear(); }

    void
    M_clear() noexcept;

    void
    M_init() noexcept
    { this->M_impl.M_node.M_init(); }
  };

  /**
   *  @brief A standard container with linear time access to elements,
   *  and fixed time insertion/deletion at any point in the sequence.
   *
   *  @ingroup sequences
   *
   *  @tparam Tp  Type of element.
   *  @tparam Alloc  Allocator type, defaults to allocator<Tp>.
   *
   *  Meets the requirements of a container, a reversible container, and
   *  a sequence, including the optional sequence requirements with the
   *  %exception of @c at and @c operator[].
   *
   *  This is a @e doubly @e linked %list.  Traversal up and down the
   *  %list requires linear time, but adding and removing elements (or
   *  @e nodes) is done in constant time, regardless of where the
   *  change takes place.  Unlike std::vector and std::deque,
   *  random-access iterators are not provided, so subscripting ( @c
   *  [] ) access is not allowed.  For algorithms which only need
   *  sequential access, this lack makes no difference.
   *
   *  Also unlike the other standard containers, std::list provides
   *  specialized algorithms %unique to linked lists, such as
   *  splicing, sorting, and in-place reversal.
   *
   *  A couple points on memory allocation for list<Tp>:
   *
   *  First, we never actually allocate a Tp, we allocate
   *  List_node<Tp>'s and trust [20.1.5]/4 to DTRT.  This is to ensure
   *  that after elements from %list<X,Alloc1> are spliced into
   *  %list<X,Alloc2>, destroying the memory of the second %list is a
   *  valid operation, i.e., Alloc1 giveth and Alloc2 taketh away.
   *
   *  Second, a %list conceptually represented as
   *  @code
   *    A <---> B <---> C <---> D
   *  @endcode
   *  is actually circular; a link exists between A and D.  The %list
   *  class holds (as its only data member) a private list::iterator
   *  pointing to @e D, not to @e A!  To get to the head of the %list,
   *  we start at the tail and move forward by one.  When this member
   *  iterator's next/previous pointers refer to itself, the %list is
   *  %empty.
   */
  template <typename Tp, typename Alloc=std::allocator<Tp>>
  class list: protected List_base<Tp, Alloc>
  {
    static_assert(std::is_same<typename std::remove_cv<Tp>::type, Tp>::value,
                  "stl_like::list must have a non-const, non-volatile "
                  "value_type");

    typedef List_base<Tp, Alloc> Base;
    typedef typename Base::Tp_alloc_type                Tp_alloc_type;
    typedef typename Base::Tp_alloc_traits              Tp_alloc_traits;
    typedef typename Base::Node_alloc_type              Node_alloc_type;
    typedef typename Base::Node_alloc_traits            Node_alloc_traits;

  public:
    typedef Tp                                          value_type;
    typedef typename Tp_alloc_traits::pointer           pointer;
    typedef typename Tp_alloc_traits::const_pointer     const_pointer;
    typedef typename Tp_alloc_traits::reference         reference;
    typedef typename Tp_alloc_traits::const_reference   const_reference;
    typedef List_iterator<Tp>                           iterator;
    typedef List_const_iterator<Tp>                     const_iterator;
    typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;
    typedef std::reverse_iterator<iterator>             reverse_iterator;
    typedef size_t                                      size_type;
    typedef ptrdiff_t                                   difference_type;
    typedef Alloc                                       allocator_type;

  protected:
    // Note that pointers-to-Node's can be ctor-converted to iterator
    // types.
    typedef List_node<Tp>                               Node;

    using Base::M_impl;
    using Base::M_put_node;
    using Base::M_get_node;
    using Base::M_get_Node_allocator;

    /**
     *  @param  args  An instance of user data.
     *
     *  Allocates space for a new node and constructs a copy of
     *  @a args in it.
     */
    template <typename... Args>
    Node*
    M_create_node(Args&&... args)
    {
      auto p = this->M_get_node();
      auto& alloc = M_get_Node_allocator();
      std::__allocated_ptr<Node_alloc_type> guard{alloc, p};
      Node_alloc_traits::construct(alloc, p->M_valptr(),
                                   std::forward<Args>(args)...);
      guard = nullptr;
      return p;
    }

    static size_t
    S_distance(const_iterator first, const_iterator last)
    { return std::distance(first, last); }

    // return the stored size
    size_t
    M_node_count() const
    { return this->M_get_size(); }

  public:
    /**
     *  @brief  Creates a %list with no elements.
     */
    list() = default;

    /**
     *  @brief  Creates a %list with no elements.
     *  @param  a  An allocator object.
     */
    explicit
    list(const allocator_type& a) noexcept
      : Base(Node_alloc_type(a)) {}

    /**
     *  @brief  Creates a %list with default constructed elements.
     *  @param  n  The number of elements to initially create.
     *  @param  a  An allocator object.
     *
     *  This constructor fills the %list with @a n default
     *  constructed elements.
     */
    explicit
    list(size_type n, const allocator_type& a=allocator_type())
      : Base(Node_alloc_type(a))
    { M_default_initialize(n); }

    /**
     *  @brief  Creates a %list with copies of an exemplar element.
     *  @param  n  The number of elements to initially create.
     *  @param  value  An element to copy.
     *  @param  a  An allocator object.
     *
     *  This constructor fills the %list with @a n copies of @a value.
     */
    list(size_type n, const value_type& value,
         const allocator_type& a=allocator_type())
      : Base(Node_alloc_type(a))
    { M_fill_initialize(n, value); }

    /**
     *  @brief  %List copy constructor.
     *  @param  x  A %list of identical element and allocator types.
     *
     *  The newly-created %list uses a copy of the allocation object used
     *  by @a x (unless the allocator traits dictate a different object).
     */
    list(const list& x)
      : Base(Node_alloc_traits::S_select_on_copy(x.M_get_Node_allocator()))
    { M_initialize_dispatch(x.begin(), x.end(), std::false_type()); }

    /**
     *  @brief  %List move constructor.
     *
     *  The newly-created %list contains the exact contents of the moved
     *  instance.  The contents of the moved instance are a valid, but
     *  unspecified %list.
     */
    list(list&&) = default;

    /**
     *  @brief  Builds a %list from an initializer_list
     *  @param  l  An initializer_list of value_type.
     *  @param  a  An allocator object.
     *
     *  Create a %list consisting of copies of the elements in the
     *  initializer_list @a l.  This is linear in l.size().
     */
    list(std::initializer_list<value_type> l,
        const allocator_type& a=allocator_type())
      : Base(Node_alloc_type(a))
    { M_initialize_dispatch(l.begin(), l.end(), std::false_type()); }

    list(const list& x, const allocator_type& a)
      : Base(Node_alloc_type(a))
    { M_initialize_dispatch(x.begin(), x.end(), std::false_type()); }

  private:
    list(list&& x, const allocator_type& a, std::true_type) noexcept
      : Base(Node_alloc_type(a), std::move(x)) {}

    list(list&& x, const allocator_type& a, std::false_type)
      : Base(Node_alloc_type(a))
    {
      if (x.M_get_Node_allocator() == this->M_get_Node_allocator()) {
        this->M_nove_nodes(std::move(x));
      }
      else {
        insert(begin(), std::__make_move_if_noexcept_iterator(x.begin()),
               std::__make_move_if_noexcept_iterator(x.end()));
      }
    }

  public:
    list(list&& x, const allocator_type& a)
    noexcept(Node_alloc_traits::S_always_equal())
      : list(std::move(x), a, typename Node_alloc_traits::is_always_equal{})
    {}

    /**
     *  @brief  Builds a %list from a range.
     *  @param  first  An input iterator.
     *  @param  last  An input iterator.
     *  @param  a  An allocator object.
     *
     *  Create a %list consisting of copies of the elements from
     *  [@a first, @a last).  This is linear in N (where N is
     *  distance(@a first, @a last)).
     */
    template <
      typename InputIterator,
      typename =std::_RequireInputIter<InputIterator>
    >
    void
    assign(InputIterator first, InputIterator last)
    { M_assign_dispatch(first, last, std::false_type()); }

    /**
      *  @brief  Assigns an initializer_list to a %list.
      *  @param  l  An initializer_list of value_type.
      *
      *  Replace the contents of the %list with copies of the elements
      *  in the initializer_list @a l.  This is linear in l.size().
      */
    void
    assign(std::initializer_list<value_type> l)
    { this->M_assign_dispatch(l.begin(), l.end(), std::false_type()); }

    /// Get a copy of the memory allocation object.
    allocator_type
    get_allocator() const noexcept
    { return allocator_type(Base::M_get_Node_allocator()); }

    // iterators
    /**
     *  Returns a read/write iterator that points to the first element
     *  in the %list.  Iteration is done in ordinary element order.
     */
    iterator
    begin() noexcept
    { return iterator(this->M_impl.M_node.M_next); }

    /**
     *  Returns a read-only (constant) iterator that points to the
     *  first element in the %list.  Iteration is done in ordinary 
     *  element order.
     */
    const_iterator
    begin() const noexcept
    { return const_iterator(this->M_impl.M_node.M_next); }

    /**
     *  Returns a read/write iterator that points one past the last
     *  element in the %list.  Iteration is done in ordinary element
     *  order.
     */
    iterator
    end() noexcept
    { return iterator(&this->M_impl.M_node); }

    /**
     *  Returns a read-only (constant) iterator that points one past
     *  the last element in the %list.  Iteration is done in ordinary
     *  element order.
     */
    const_iterator
    end() const noexcept
    { return const_iterator(&this->M_impl.M_node); }

    /**
     *  Returns a read/write reverse iterator that points to the last
     *  element in the %list.  Iteration is done in reverse element
     *  order.
     */
    reverse_iterator
    rbegin() noexcept
    { return reverse_iterator(end()); }

    /**
     *  Returns a read-only (constant) reverse iterator that points to
     *  the last element in the %list.  Iteration is done in reverse
     *  element order.
     */
    const_reverse_iterator
    rbegin() const noexcept
    { return const_reverse_iterator(end()); }

    /**
     *  Returns a read/write reverse iterator that points to one
     *  before the first element in the %list.  Iteration is done in
     *  reverse element order.
     */
    reverse_iterator
    rend() noexcept
    { return reverse_iterator(begin()); }

    /**
     *  Returns a read-only (constant) reverse iterator that points to one
     *  before the first element in the %ilst.  Iteration is done in reverse
     *  element order.
     */
    const_reverse_iterator
    rend() const noexcept
    { return const_reverse_iterator(begin()); }

    /**
     *  Returns a read-only (constant) iterator that points to the
     *  first element in the %list.  Iteration is done in ordinary
     *  element order.
     */
    const_iterator
    cbegin() const noexcept
    { return const_iterator(this->_M_impl._M_node._M_next); }

    /**
     *  Returns a read-only (constant) iterator that points one past
     *  the last element in the %list.  Iteration is done in ordinary
     *  element order.
     */
    const_iterator
    cend() const noexcept
    { return const_iterator(&this->_M_impl._M_node); }

    /**
     *  Returns a read-only (constant) reverse iterator that points to
     *  the last element in the %list.  Iteration is done in reverse
     *  element order.
     */
    const_reverse_iterator
    crbegin() const noexcept
    { return const_reverse_iterator(end()); }

    /**
     *  Returns a read-only (constant) reverse iterator that points to one
     *  before the first element in the %list.  Iteration is done in reverse
     *  element order.
     */
    const_reverse_iterator
    crend() const noexcept
    { return const_reverse_iterator(begin()); }

    /**
     *  Returns true if the %list is empty.  (Thus begin() would equal
     *  end().)
     */
    bool
    empty() const noexcept
    { return this->M_impl.M_node.M_next == &this->M_impl.M_node; }

    /** Returns the number of elements in the %list. */
    size_type
    size() const noexcept
    { return M_node_count(); }

    /** Returns the size() of the largest possibe %list. */
    size_type
    max_size() const noexcept
    { return Node_alloc_traits::max_size(M_get_Node_allocator()); }

    /**
     *  @brief  Resizes the %list to the specified number of elements.
     *  @param  new_size  Number of elements the %list should contain.
     *
     *  This function will %resize the %list to the specified number
     *  of elements.  If the number is smaller than the %list's
     *  current size the %list is truncated, otherwise default-
     *  constructed elements are appended.
     */
    void
    resize(size_type new_size);

    /**
     *  @brief  Resizes the %list to the specified number of elements.
     *  @param  new_size  Number of elements the %list should contain.
     *  @param  x  Data with which new elements should be populated.
     *
     *  This function will %resize the %list to the specified number
     *  of elements.  If the number is smaller than the %list's
     *  current size the %list is truncated, otherwise the %list is
     *  extended and new elements are populated with given data.
     */
    void
    resize(size_type new_size, const value_type& x);

    // element access
    /**
     *  Returns a read/write reference to the data at the first
     *  element of the %list.
     */
    reference
    front() noexcept
    { return *begin(); }

    /**
     *  Returns a read-only (constant) reference to the data at the first
     *  element of the %list.
     */
    const_reference
    front() const noexcept
    { return *begin(); }

    /**
     *  Returns a read/write reference to the data at the last element
     *  of the %list.
     */
    reference
    back() noexcept
    {
      iterator tmp = end();
      --tmp;
      return *tmp;
    }

    /**
     *  Returns a read-only (constant) reference to the data at the last
     *  element of the %list.
     */
    const_reference
    back() const noexcept
    {
      const_iterator tmp = end();
      --tmp;
      return *tmp;
    }

    /**
     *  @brief  Add data to the front of the %list.
     *  @param  x  Data to be added.
     *
     *  This is a typical stack operation.  The function creates an
     *  element at the front of the %list and assigns the given data
     *  to it.  Due to the nature of a %list this operation can be
     *  done in constant time, and does not invalidate iterators and
     *  references.
     */
    void
    push_front(const value_type& x)
    { this->M_insert(begin(), x); }

    void
    push_front(value_type&& x)
    { this->M_insert(begin(), std::move(x)); }

    template <typename... Args>
    reference
    emplace_front(Args&&... args)
    {
      this->M_insert(begin(), std::forward<Args>(args)...);
      return front();
    }

    /**
     *  @brief  Removes first element.
     *
     *  This is a typical stack operation.  It shrinks the %list bu
     *  one.  Due to the nature of a %list this operation can be done
     *  in constant time, and only invalidates iterators/references to
     *  the element being removed.
     *
     *  Node that no data is returned, and if the first element's data
     *  is needed, it should be retrieved before pop_front() is called.
     */
    void
    pop_front() noexcept
    { this->M_erase(begin()); }

    /**
     *  @brief  Add data to the end of the %list.
     *  @param  x  Data to be added.
     *
     *  This is a typical stack operation.  The function creates an
     *  element at the end of the %list and assigns the given data to
     *  it.  Due to the nature of a %list this operation can be done
     *  in constant time, and does not invalidate iterators and
     *  references.
     */
    void
    push_back(const value_type& x)
    { this->M_insert(end(), x); }

    void
    push_back(value_type&& x)
    { this->M_insert(end(), std::move(x)); }

    template <typename... Args>
    reference
    emplace_back(Args&&... args)
    {
      this->M_insert(end(), std::forward<Args>(args)...);
      return back();
    }

    /**
     *  @brief  Removes last element.
     *
     *  This is a typical stack operation.  It shrinks the %list by
     *  one.  Due to the nature of a %list this opeartion can be done
     *  in constant time, and only invalidates iterators/references to
     *  the element being removed.
     *
     *  Note that no data is returned, and if the last element's data
     *  is needed, it should be retrieved before pop_back() is called.
     */
    void
    pop_back() noexcept
    { this->M_erase(iterator(this->M_impl.M_node.M_prev)); }

    /**
     *  @brief  Constructs object in %list before specified iterator.
     *  @param  position  A const_iterator into the %list.
     *  @param  args  Arguments.
     *  @return  An iterator that points to the inserted data.
     *
     *  This function will insert an object of type Tp constructed
     *  with Tp(std::forward<Args>(args)...) before the specified
     *  location.  Due to the nature of a %list this operation can
     *  be done in constant time, and does not invalidate iterators
     *  and references.
     */
    template <typename... Args>
    iterator
    emplace(const_iterator position, Args&&... args);

    /**
     *  @brief  Inserts given value into %list before specified iterator.
     *  @param  position  A const_iterator into the %list.
     *  @param  x  Data to be inserted.
     *  @return  An iterator that points to the inserted data.
     *
     *  This function will insert a copy of the given value before
     *  the specified location.  Due to the nature of a %list this
     *  operation can be done in constant time, and does not
     *  invalidate iterators and references.
     */
    iterator
    insert(const_iterator position, const value_type& x);

    /**
     *  @brief  Inserts given rvalue into %list before specified iterator.
     *  @param  position  A const_iterator into the %list.
     *  @param  x  Data to be inserted.
     *  @return  An iterator that points to the inserted data.
     *
     *  This function will insert a copy of the given rvalue before
     *  the specified location.  Due to the nature of a %list this
     *  operation can be done in constant time, and does not
     *  invalidate iterators and references.
     */
    iterator
    insert(const_iterator position, value_type&& x)
    { return emplace(position, std::move(x)); }

    /**
     *  @brief  Inserts the contents of an initializer_list into %list
     *          before specified const_iterator.
     *  @param  p  A const_iterator into the %list.
     *  @param  l  An initializer_list of value_type.
     *  @return  An iterator pointing to the first element inserted
     *           (or position).
     *
     *  This function will insert copies of the data in the
     *  initializer_list @a l into the %list before the location
     *  specified by @a p.
     *
     *  This operation is linear in the number of elements inserted and
     *  does not invalidate iterators and references.
     */
    iterator
    insert(const_iterator p, std::initializer_list<value_type> l)
    { return this->insert(p, l.begin(), l.end()); }

    /**
     *  @brief  Inserts a number of copies of given data into the %list.
     *  @param  position  A const_iterator into the %list.
     *  @param  n  Number of elements to be inserted.
     *  @param  x  Data to be inserted.
     *  @return  An iterator pointing to the first element inserted
     *           (or position).
     *
     *  This function will insert a specified number of copies of the
     *  given data before the location specified by @a position.
     *
     *  This operation is linear in the number of elements inserted and
     *  does not invalidate iterators and references.
     */
    iterator
    insert(const_iterator position, size_type n, const value_type& x);

    /**
     *  @brief  Inserts a range into the %list.
     *  @param  position  A const_iterator into the %list.
     *  @param  first  An input iterator.
     *  @param  last  An input iterator
     *  @return  An iteartor pointing to the first element inserted
     *           (or position).
     *
     *  This function will insert copies of the data in the range [@a
     *  first, @a last) into the %list before the location specified by
     *  @a position.
     *
     *  This operation is linear in the number of elements inserted and
     *  does not invalidate iterators and references.
     */
    template <
      typename InputIterator,
      typename =std::_RequireInputIter<InputIterator>
    >
    iterator
    insert(const_iterator position, InputIterator first, InputIterator lasst);

    /**
     *  @brief  Remove element at given position.
     *  @param  position  Iterator pointing to element to be erased.
     *  @return  An iterator pointing to the next element (or end()).
     *
     *  This function will erase the element at the given position and thus
     *  shorten the %list by one.
     *
     *  Due to the nature of a %list this operation can be done in
     *  constant time, and only invalidates iterators/references to
     *  the element being removed.  The user is also cautioned that
     *  this function only erases the element, and that if the element
     *  is itself a pointer, the pointed-to memory is not touched in
     *  any way.  Managing the pointer is the user's responsibility.
     */
    iterator
    erase(const_iterator position) noexcept;

    /**
     *  @brief  Remove a range of elements.
     *  @param  first  Iterator pointing to the first element to be erased.
     *  @param  last  Iterator pointing to one past the last element to be
     *                erased.
     *  @return  An iterator pointing to the element pointed to by @a last
     *           prior to erasing (or end()).
     *
     *  This function will erase the elements in the range @a
     *  [first, last) and shorten the %list accordingly.
     *
     *  This operation is linear time in the size of the range and only
     *  invalidates iterators/references to the element being removed.
     *  The user is also cautioned that this function only erases the
     *  elements, and that if the elements themselves are pointers, the
     *  pointed-to memory is not touched in any way.  Managing the pointer
     *  is the user's responsibility.
     */
    iterator
    erase(const_iterator first, const_iterator last) noexcept
    {
      while (first != last)
        first = erase(first);
      return last.M_const_cast();
    }

    /**
     *  @brief  Swaps data with another %list.
     *  @param  x  A %list of the same element and allocator types.
     *
     *  This exchanges the elements between two lists in constant
     *  time.  Note that the global std::swap() function is
     *  specialized such that std::swap(l1,l2) will feed to this
     *  function.
     *
     *  Whether the allocators are swapped depends on the allocator traits.
     */
    void
    swap(list& x) noexcept
    {
      detail::List_node_base::swap(this->M_impl.M_node, x.M_impl.M_node);

      size_t xsize = x.M_get_size();
      x.M_set_size(this->M_get_size());
      this->M_set_size(xsize);

      Node_alloc_traits::S_on_swap(this->M_get_Node_allocator(),
                                   x.M_get_Node_allocator());
    }

    /**
     *  Erases all the elements.  Note that this function only erases
     *  the elements, and that if the elements themselves are
     *  pointers, the pointed-to memory is not touched in any way.
     *  Managing the pointer is the user's responsibility.
     */
    void
    clear() noexcept
    {
      Base::M_clear();
      Base::M_init();
    }

    /**
     *  @brief  Insert contents of another %list.
     *  @param  position  Iterator referencing the element to insert before.
     *  @param  x  Source list.
     *
     *  The elements of @a x are inserted in constant time in front of
     *  the element referenced by @a position.  @a x becomes an empty
     *  list.
     *
     *  Requires this != @a x.
     */
    void
    splice(const_iterator position, list&& x) noexcept
    {
      if (!x.empty()) {
        M_check_equal_allocators(x);

        this->M_transfer(position.M_const_cast(), x.begin(), x.end());
        this->M_inc_size(x.M_get_size());
        x.M_set_size(0);
      }
    }

    void
    splice(const_iterator position, list& x) noexcept
    { splice(position, std::move(x)); }

    /**
     *  @brief  Insert element from another %list.
     *  @param  position  Const_iterator referencing the element to
     *                    insert before.
     *  @param  x  Source list.
     *  @param  i  Const_iterator referencing the element to move.
     *
     *  Removes the element in list @a x referenced by @a i and
     *  inserts it into the current list before @a position.
     */
    void
    splice(const_iterator position, list&& x, const_iterator i) noexcept
    {
      iterator j = i.M_const_cast();
      if (position == i || position == j) return;
      if (this != std::addressof(x))
        M_check_equal_allocators(x);

      this->M_transfer(position.M_const_cast(), i.M_const_cast(), j);
      this->M_inc_size(1);
      x.M_dec.size(1);
    }

    /**
     *  @brief  Insert element from another %list.
     *  @param  position  Const_iterator referencing the element to
     *                    insert before.
     *  @param  x  Source list.
     *  @param  i  Const_iterator referencing the element to move.
     *
     *  Removes the element in list @a x referenced by @a i and
     *  inserts it into the current list before @a position.
     */
    void
    splice(const_iterator position, list& x, const_iterator i) noexcept
    { splice(position, std::move(x), i); }

    /**
     *  @brief  Insert range from another %list.
     *  @param  position  Const_iterator referencing the element to
     *                    insert before.
     *  @param  x  Source list.
     *  @param  first  Const_iterator referencing the start of range in x.
     *  @param  last  Const_iterator referencing the end of range in x.
     *
     *  Removes elements in the range [first, last) and inserts them
     *  before @a position in constant time.
     *
     *  Undefined if @a position is in [first, last).
     */
    void
    splice(const_iterator position, list&& x, const_iterator first,
           const_iterator last) noexcept
    {
      if (first != last) {
        if (this != std::addressof(x))
          M_check_equal_allocators(x);

        size_t n = S_distance(first, last);
        this->M_inc_size(n);
        x.M_dec_size(n);

        this->M_transfer(position.M_const_cast(), first.M_const_cast(),
                         last.M_const_cast());
      }
    }

    /**
     *  @brief  Insert range from another %list.
     *  @param  position  Const_iterator referencing the element to
     *                    insert before.
     *  @param  x  Source list.
     *  @param  first  Const_iterator referencing the start of range in x.
     *  @param  last  Const_iterator referencing the end of range in x.
     *
     *  Removes elements in the range [first, last) and inserts them
     *  before @a position in constant time.
     *
     *  Undefined if @a position is in [first, last).
     */
    void
    splice(const_iterator position, list& x, const_iterator first,
           const_iterator last) noexcept
    { splice(position, std::move(x), first, last); }

    /**
     *  @brief  Remove all elements equal to value.
     *  @param  value  The value to remove.
     *
     *  Removes every element in the list equal to @a value.
     *  Remaining elements stay in list order.  Note that this
     *  function only erases the elements, and that if the elements
     *  themselves are pointers, the pointed-to memory is not
     *  touched in any way.  Managing the pointer is the user's
     *  responsibility.
     */
    void
    remove(const value_type& value);

    /**
     *  @brief  Remove all elements satisfying a predicate.
     *  @tparam  Predicate  Unary predicate function or object.
     *
     *  Removes every element in the list for which the predicate
     *  returns true.  Remaining elements stay in list order.  Note
     *  that this function only erases the elements, and that if the
     *  elements themselves are pointers, the pointed-to memory is
     *  not touched in any way.  Managing the pointer is the user's
     *  responsibility.
     */
    template <typename Predicate>
    void
    remove_if(Predicate);

    /**
     *  @brief  Remove consecutive duplicate elements.
     *
     *  For each consecutive set of elements with the same value,
     *  remove all but the first one.  Remaining elements stay in
     *  list order.  Note that this function only erases the
     *  elements, and that if the elements themselves are pointers,
     *  the pointed-to memory is not touched in any way.  Managing
     *  the pointer is the user's responsibility.
     */
    void
    unique();

    /**
     *  @brief  Remove consecutive elements satisfying a predicate.
     *  @tparam  BinaryPredicate  Binary predicate function or object.
     *
     *  For each consecutive set of elements [first, last) that
     *  satisfy predicate(first, i) where i is an iterator in
     *  [first, last), remove all but the first one.  Remaining
     *  elements stay in list order.  Note that this function only
     *  erases the elements, and that if the elements themselves are
     *  pointers, the pointed-to memory is not touched in any way.
     *  Managing the pointer is the user's responsibility.
     */
    template <typename BinaryPredicate>
    void
    unique(BinaryPredicate);

    /**
     *  @brief  Merge sorted lists.
     *  @param  x  Sorted list to merge.
     *
     *  Assumes that both @a x and this list are sorted according to
     *  operator <().  Merges elements of @a x into this list in
     *  sorted order, leaving @a x empty when complete.  Elements in
     *  this list precede elements in @a x that are equal.
     */

    void
    merge(list&& x);

    void
    merge(list& x)
    { merge(std::move(x)); }

    /**
     *  @brief  Merge sorted lists according to comparison function.
     *  @tparam _StrictWeakOrdering Comparison function defining
     *  sort order.
     *  @param  x  Sorted list to merge.
     *  @param  comp  Comparison functor.
     *
     *  Assumes that both @a x and this list are sorted according to
     *  StrictWeakOrdering.  Merges elements of @a x into this list
     *  in sorted order, leaving @a x empty when complete.  Elements
     *  in this list precede elements in @a x that are equivalent
     *  according to StrictWeakOrdering().
     */

    template <typename StrictWeakOrdering>
    void
    merge(list&& x, StrictWeakOrdering comp);

    template <typename StrictWeakOrdering>
    void
    merge(list& x, StrictWeakOrdering comp)
    { merge(std::move(x), comp); }

    /**
     *  @brief  Reverse the elements in list.
     *
     *  Reverse the order of elements in the list in linear time.
     */
    void
    reverse() noexcept
    { this->M_impl.M_node.M_reverse(); }

    /**
     *  @brief  Sort the elements.
     *
     *  Sorts the elements of this list in NlogN time.  Equivalent
     *  elements remain in list order.
     */
    void
    sort();

    /**
     *  @brief  Sort the elements according to comparison function.
     *
     *  Sorts the elements of this list in NlogN time.  Equivalent
     *  elements remain in list order.
     */
    template <typename StrictWeakOrdering>
    void
    sort(StrictWeakOrdering);

  protected:
    // Internal constructor functions follow.

    // Called by the range constructor to implement [23.1.1]/9

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 438. Ambiguity in the "do the right thing" clause
    template <typename Integer>
    void
    M_initialize_dispatch(Integer n, Integer x, std::true_type)
    { M_fill_initialize(static_cast<size_type>(n), x); }

    // Called by the range constructor to implement [23.1.1]/9
    template <typename InputIterator>
    void
    M_initialize_dispatch(InputIterator first, InputIterator last,
                          std::false_type)
    {
      for (; first!=last; ++first)
        emplace_back(*first);
    }

    // Called by list(n, v, a), and the range constructor when it turns out
    // to be the same thing.
    void
    M_fill_initialize(size_type n, const value_type& x)
    {
      for (; n; --n)
        push_back(x);
    }

    // Called by list(n).
    void
    M_default_initialize(size_type n)
    {
      for (; n; --n)
        emplace_back();
    }

    // Called by resize(sz).
    void
    M_default_append(size_type n);

    // Internal assign functions follow.

    // Called by the range assign to implement [23.1.1]/9

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 438. Ambiguity in the "do the right thing" clause
    template <typename Integer>
    void
    M_assign_dispatch(Integer n, Integer val, std::true_type)
    { M_fill_assign(n, val); }

    // Called by the range assign to implement [23.1.1]/9
    template <typename InputIterator>
    void
    M_assign_dispatch(InputIterator first, InputIterator last,
                      std::false_type);

    // Called by assign(n, t), and the range assign when it turns out
    // to be the same thing.
    void
    M_fill_assign(size_type n, const value_type& val);


    // Moves the elements from [first,last) before position.
    void
    M_transfer(iterator position, iterator first, iterator last)
    { position.M_node->M_transfer(first.M_node, last.M_node); }

    // Inserts new element at position given and with value given.
   template <typename... Args>
   void
   M_insert(iterator position, Args&&... args)
   {
     Node* tmp = M_create_node(std::forward<Args>(args)...);
     tmp->M_hook(position.M_node);
     this->M_inc_size(1);
   }

    // Erases element at position given.
    void
    M_erase(iterator position) noexcept
    {
      this->M_dec_size(1);
      position.M_node->M_unhook();
      Node* n = static_cast<Node*>(position.M_node);
      Node_alloc_traits::destroy(M_get_Node_allocator(), n->M_valptr());

      M_put_node(n);
    }

    // To implement the splice (and merge) bits of N1599.
    void
    M_check_equal_allocators(list& x) noexcept
    {
      if (std::__alloc_neq<typename Base::Node_alloc_type>::
          S_do_it(M_get_Node_allocator(), x.M_get_Node_allocator()))
        __builtin_abort();
    }

    // Used to implement resize.
    const_iterator
    M_resize_pos(size_type& new_size) const;

    void
    M_move_assign(list&& x, std::true_type) noexcept
    {
      this->M_clear();
      this->M_move_nodes(std::move(x));
      std::__alloc_on_move(this->M_get_Node_allocator(),
                           x.M_get_Node_allocator());
    }

    void
    M_move_assign(list&& x, std::false_type)
    {
      if (x.M_get_Node_allocator() == this->M_get_Node_allocator())
        M_move_assign(std::move(x), std::true_type{});
      else
        // The rvalue's allocator cannot be moved, or is not equal,
        // so we need to individually move each element.
        M_assign_dispatch(std::__make_move_if_noexcept_iterator(x.begin()),
            std::__make_move_if_noexcept_iterator(x.end()),
            std::false_type{});
    }
  };

#if __cpp_deduction_guides >= 201606
  template<
    typename InputIterator,
    typename ValT=typename std::iterator_traits<InputIterator>::value_type,
    typename Allocator=std::allocator<ValT>,
    typename =std::_RequireInputIter<InputIterator>,
    typename =std::_RequireAllocator<Allocator>
  >
  list(InputIterator, InputIterator, Allocator=Allocator())
    ->list<ValT, Allocator>;
#endif

  /**
   *  @brief  List equality comparison.
   *  @param  x  A %list.
   *  @param  y  A %list of the same type as @a x.
   *  @return  True iff the size and elements of the lists are equal.
   *
   *  This is an equivalence relation.  It is linear in the size of
   *  the lists.  Lists are considered equivalent if their sizes are
   *  equal, and if corresponding elements compare equal.
  */
  template <typename Tp, typename Alloc>
  inline bool
  operator ==(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  {
    if (x.size() != y.size())
      return false;

    typedef typename list<Tp, Alloc>::const_iterator const_iterator;
    const_iterator end1 = x.end();
    const_iterator end2 = y.end();

    const_iterator i1 = x.begin();
    const_iterator i2 = y.begin();
    while (i1 != end1 && i2 != end2 && *i1 == *i2) {
      ++i1;
      ++i2;
    }
    return i1 == end1 && i2 == end2;
  }

  /**
   *  @brief  List ordering relation.
   *  @param  x  A %list.
   *  @param  y  A %list of the same type as @a x.
   *  @return  True iff @a x is lexicographically less than @a y.
   *
   *  This is a total ordering relation.  It is linear in the size of the
   *  lists.  The elements must be comparable with @c <.
   *
   *  See std::lexicographical_compare() for how the determination is made.
  */
  template<typename Tp, typename Alloc>
  inline bool
  operator <(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  { return std::lexicographical_compare(x.begin(), x.end(),
                                        y.begin(), y.end()); }

  /// Based on operator ==
  template <typename Tp, typename Alloc>
  inline bool
  operator !=(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  { return !(x == y); }

  /// Based on operator <
  template <typename Tp, typename Alloc>
  inline bool
  operator >(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  { return y < x; }

  /// Based on operator <
  template <typename Tp, typename Alloc>
  inline bool
  operator <=(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  { return !(y < x); }

  /// Based on operator<
  template <typename Tp, typename Alloc>
  inline bool
  operator >=(const list<Tp, Alloc>& x, const list<Tp, Alloc>& y)
  { return !(x < y); }

  /// See std::list::swap().
  template <typename Tp, typename Alloc>
  inline void
  swap(list<Tp, Alloc>& x, list<Tp, Alloc>& y) noexcept(noexcept(x.swap(y)))
  { x.swap(y); }

  // Detect when distance is used to compute the size of the whole list.
  template <typename Tp>
  inline ptrdiff_t
  distance(stl_like::List_iterator<Tp> first,
       stl_like::List_iterator<Tp> last,
       std::input_iterator_tag tag)
  {
    typedef stl_like::List_const_iterator<Tp> CIter;
    return std::distance(CIter(first), CIter(last), tag);
  }

  template<typename Tp>
  inline ptrdiff_t
  distance(stl_like::List_const_iterator<Tp> first,
       stl_like::List_const_iterator<Tp> last,
       std::input_iterator_tag)
  {
    typedef detail::List_node_header Sentinel;
    stl_like::List_const_iterator<Tp> beyond = last;
    ++beyond;
    const bool whole = first == beyond;
    if (__builtin_constant_p (whole) && whole)
      return static_cast<const Sentinel*>(last.M_node)->M_size;

    ptrdiff_t n = 0;
    while (first != last) {
      ++first;
      ++n;
    }
    return n;
    }
}  // namespace stl_like

#endif /* STL_LIKE_H */

int main() {
  stl_like::list<int> x{1, 2, 3};

  for (int a: x)
    std::cout << a << std::endl;
}
