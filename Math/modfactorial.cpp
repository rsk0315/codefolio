class modfactorial {
  std::vector<intmax_t> fact, fact_inv;
  intmax_t mod;

public:
  modfactorial(intmax_t N, intmax_t mod): mod(mod) {
    fact.resize(N+1);
    fact_inv.resize(N+1);
    fact[0] = 1;
    for (intmax_t i = 1; i <= N; ++i)
      fact[i] = fact[i-1] * i % mod;

    fact_inv[N] = modinv(fact[N], mod);
    for (intmax_t i = N; i--;)
      fact_inv[i] = fact_inv[i+1] * (i+1) % mod;
  }

  intmax_t operator ()(intmax_t k) const {
    return fact[k];
  }

  intmax_t inverse(intmax_t k) const {
    return fact_inv[k];
  }
};
