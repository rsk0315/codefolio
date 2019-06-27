template <class Tp>
Tp& chmin(Tp& dst, const Tp& src) {
  if (src < dst) dst = src;
  return dst;
}

template <class Tp>
Tp& chmax(Tp& dst, const Tp& src) {
  if (dst < src) dst = src;
  return dst;
}
