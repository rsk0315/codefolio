template <typename Tp>
class congruences {
  Tp M_sol;
  Tp M_mod = 1;

public:
  congruences() {}

  std::pair<Tp, Tp> append(Tp a, Tp q) {
    if (M_mod == 1) {
      M_sol = a % (M_mod = q);
    } else {
      Tp x = (modsub(a, M_sol, q) * modinv(M_mod, q)) % q;
      M_sol += x * M_mod;
      M_mod *= q;
    }
    return {M_sol, M_mod};
  }

  std::pair<Tp, Tp> get() const {
    return {M_sol, M_mod};
  }
};
