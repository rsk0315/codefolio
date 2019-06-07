template <typename Tp, typename Fn>
std::vector<Tp> transition(std::vector<size_t> const& dst,
                           std::vector<Tp> const& value,
                           Fn f, uintmax_t k) {

  size_t n = dst.size();
  std::vector<Tp> res_v(n, f.identity);
  std::vector<size_t> res_d(n);
  std::iota(res_d.begin(), res_d.end(), 0);
  std::vector<Tp> dbl_v = value;
  std::vector<size_t> dbl_d = dst;
  for (; k; k >>= 1) {
    if (k & 1) {
      std::vector<size_t> tmp_d = res_d;
      for (size_t i = 0; i < n; ++i) {
        res_v[i] = f(res_v[i], dbl_v[tmp_d[i]]);
        res_d[i] = dbl_d[tmp_d[i]];
      }
    }
    {
      std::vector<Tp> tmp_v = dbl_v;
      std::vector<size_t> tmp_d = dbl_d;
      for (size_t i = 0; i < n; ++i) {
        dbl_v[i] = f(dbl_v[i], tmp_v[tmp_d[i]]);
        dbl_d[i] = tmp_d[tmp_d[i]];
      }
    }
  }
  return res_v;
}
