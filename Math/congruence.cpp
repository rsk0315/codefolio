template <class Tp>
class congruences {
  Tp sol, mod=1;

public:
  congruences() {}

  std::pair<Tp, Tp> append(Tp a, Tp q) {
    if (mod == 1) {
      sol = a % (mod = q);
    } else {
      Tp x = (modsub(a, sol, q) * modinv(mod, q)) % q;
      sol += x * mod;
      mod *= q;
    }
    return {sol, mod};
  }

  std::pair<Tp, Tp> get() const {
    return {sol, mod};
  }
};
