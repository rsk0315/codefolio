namespace bit {
template <typename Tp>
int log2p1(Tp n) {
  if (n == 0) return 0;
  return (CHAR_BIT * sizeof(long long)) - __builtin_clzll(n);
}
}  // bit::
