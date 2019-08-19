template <typename Tp>
class range {
public:
  using value_type = Tp;
  using difference_type = typename std::make_signed<value_type>::type;

private:
  value_type M_start = 0;
  value_type M_stop;
  difference_type M_stride = 1;
  bool M_fail = false;

public:
  range(value_type stop): M_stop(stop) {}
  range(value_type start, value_type stop, difference_type stride = 1):
    M_start(start), M_stop(stop), M_stride(stride)
  {}

  class iterator {
  public:
    using value_type = Tp;
    using difference_type = typename std::make_signed<value_type>::type;

  private:
    value_type M_start, M_stop;
    difference_type M_stride;
    value_type M_index;
    bool M_fail = false;
    static constexpr value_type S_min = std::numeric_limits<value_type>::min();
    static constexpr value_type S_max = std::numeric_limits<value_type>::max();

  public:
    iterator(value_type start, value_type stop, difference_type stride,
             value_type index):
      M_start(start), M_stop(stop), M_stride(stride), M_index(index)
    {}
    value_type operator *() const { return M_index; }
    iterator& operator ++() {
      if ((0 < M_stride && __builtin_add_overflow(M_index, M_stride, &M_index))
          || (M_stride < 0 && __builtin_sub_overflow(M_index, -M_stride, &M_index)))
        M_fail = true;
      return *this;
    }
    bool operator !=(iterator const& other) const {
      if (M_fail || other.M_fail) return false;

      if (0 < M_stride && M_stop == S_min) return !M_fail;
      if (M_stride < 0 && M_stop == S_max) return !M_fail;
      return ((0 < M_stride && M_index < M_stop)
              || (M_stride < 0 && M_stop < M_index));
    }
  };

  iterator begin() const { return iterator(M_start, M_stop, M_stride, M_start); }
  iterator end() const { return iterator(M_start, M_stop, M_stride, M_stop); }
};
