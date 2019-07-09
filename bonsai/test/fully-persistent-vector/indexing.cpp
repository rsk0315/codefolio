#include <cstdio>
#include <vector>

static constexpr size_t S_log_branch = 2;
static constexpr size_t S_mask = (1u << (S_log_branch)) - 1;
void neko(size_t k) {
  std::vector<size_t> res;
  fprintf(stderr, "%zu: ", k);
  size_t height = 0;

  size_t j = 0;
  while (j < k) {
    ++height;
    ++j <<= S_log_branch;
  }
  while (height--) {
    j >>= S_log_branch;
    size_t c = (k - j) >> (height * S_log_branch);
    res.push_back(c);

    k -= (c+1) << (height * S_log_branch);
  }
  for (size_t i = 0; i < res.size(); ++i)
    printf("%zu%c", res[i], i+1<res.size()? ' ': '\n');
}

int main() {
  size_t n = 100;
  for (size_t i = 1; i <= n; ++i)
    neko(i);
}
