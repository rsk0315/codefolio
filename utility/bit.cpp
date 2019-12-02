namespace bit {
template <typename Tp>
int log2p1(Tp n) {
  if (n == 0) return 0;
  return (CHAR_BIT * sizeof(long long)) - __builtin_clzll(n);
}

template <typename Tp>
Tp ceil2(Tp n) {
  if (n == 0) return 1;
  return Tp{1} << log2p1(n-1);
}

template <typename Tp>
int ctz(Tp n) {
  return __builtin_ctzll(n);
}
}  // bit::
