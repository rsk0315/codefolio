#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <array>
#include <deque>

class suffix_array {
public:
  using size_type = size_t;
  using value_type = typename std::string::value_type;
  static size_type constexpr S_sigma = 256;

private:
  std::vector<size_type> M_c;

  static void S_induced_sort(std::string const& str,
                             std::vector<bool> const& s_type,
                             std::vector<bool> const& lms_type,
                             std::vector<size_type>& sa) {

    size_type n = str.length();
    std::array<size_type, S_sigma> tail = {};
    for (size_type i = 0; i <= n; ++i)
      ++tail[size_type(str[i])];
    for (size_type i = 1; i < S_sigma; ++i)
      tail[i] += tail[i-1];

    std::array<size_type, S_sigma> head = {};
    for (size_type i = 1; i < S_sigma; ++i)
      head[i] = tail[i-1]--;

    for (size_type i = 0; i <= n; ++i) {
      if (!lms_type[i]) continue;
      sa[tail[size_type(str[i])]--] = i;
    }

    for (size_type i = 0; i <= n; ++i) {
      if (sa[i]+1 == 0) continue;
      size_type j = sa[i];
      if (!j) continue;
      if (!s_type[j-1]) sa[head[size_type(str[j-1])]++] = j-1;
      if (lms_type[j] && str[j]) sa[++tail[size_type(str[j])]] = -1;
    }

    for (size_type i = n+1; i--;) {
      if (sa[i]+1 == 0) continue;
      size_type j = sa[i];
      if (!j) continue;
      if (s_type[j-1]) sa[tail[size_type(str[j-1])]--] = j-1;
    }
  }

  static std::vector<size_t> S_build(std::string const& str) {
    size_type n = str.length();

    // classify suffices into {L, S, LMS} type
    std::vector<bool> s_type(n+1), lms_type(n+1);
    s_type[n] = true;
    for (size_type i = n; i--;)
      if (str[i] < str[i+1]) s_type[i] = true;
    for (size_type i = n; i--;)
      if (!s_type[i] && s_type[i+1]) lms_type[i+1] = true;

    // sort LMS_type suffices
    std::string lms;
    for (size_type i = 0; i <= n; ++i) {
      if (!lms_type[i]) continue;
      fprintf(stderr, "%zu\n", i);
      lms += str[i];
    }
    std::vector<bool> tmp(lms.length()+1, true);
    fprintf(stderr, "lms: %s\n", lms.c_str());
    std::vector<size_type> minisa(lms.length()+1, -1);
    S_induced_sort(lms, tmp, tmp, minisa);

    // induced sort
    std::vector<size_type> sa(n+1, -1);
    S_induced_sort(str, s_type, lms_type, sa);
    
    return sa;
  }

public:
  suffix_array() = default;
  suffix_array(suffix_array const&) = default;
  suffix_array(suffix_array&&) = default;

  suffix_array(std::string const& str): M_c(S_build(str)) {}

  suffix_array& operator =(suffix_array const&) = default;
  suffix_array& operator =(suffix_array&&) = default;

  void inspect() const {
    size_type n = M_c.size();
    for (size_type i = 0; i < n; ++i)
      fprintf(stderr, "%zu%c", M_c[i], i+1<n? ' ':'\n');
  }
};

int main() {
  char buf[128];
  scanf("%s", buf);
  std::string s = buf;
  suffix_array(s).inspect();
  // suffix_array("acacacadabra").inspect();
  // suffix_array("abracadabra").inspect();
  // suffix_array("mississippi").inspect();
}
