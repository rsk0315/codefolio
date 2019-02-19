template <intmax_t X=943681729, intmax_t MOD=1000000007>
class rolling_hash {
  std::vector<intmax_t> hash;
  intmax_t offset(size_t len) const {
    intmax_t res = 1;
    for (intmax_t dbl = X%MOD; len; len >>= 1) {
      if (len & 1)
        res = res*dbl % MOD;

      dbl = dbl*dbl % MOD;
    }

    return res;
  }

public:
  rolling_hash(const std::string& s): hash(s.length()+1) {
    for (size_t i = 0; i < s.length(); ++i)
      hash[i+1] = (hash[i]*X+s[i]) % MOD;
  }

  intmax_t get_base() const { return X; }
  intmax_t get_mod() const { return MOD; }

  intmax_t prefix(size_t i) const {
    // hash of prefix s[0..i-1] (inclusive)
    assert(0 < i);
    return hash[i];
  }

  intmax_t substr(size_t i, size_t j) const {
    // hash of substring s[i..j-1] (inclusive)
    assert(i < j);
    intmax_t res = hash[j];
    res = (res - offset(j-i)*hash[i]) % MOD;
    if (res < 0) res += MOD;
    return res;
  }

  intmax_t inserted(size_t i, int ch) const {
    intmax_t res = 0;
    if (i > 0) res += prefix(i);
    res = (res*X + ch) % MOD;
    size_t n = hash.size()-1;
    if (i < n) res = (res*offset(n-i) + substr(i, n)) % MOD;
    return res;
  }

  void debug() const {
    for (size_t i = 0; i < hash.size(); ++i)
      fprintf(stderr, "%jd%c", hash[i], i+1<hash.size()? ' ':'\n');
  }
};
