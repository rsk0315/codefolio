#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
#include <limits>
#include <numeric>
#include <iostream>

#include "../../utility/literals.cpp"
#include "../../DataStructure/suffix_array.cpp"

int main() {
  {
    std::vector<int> a{0, 1, 17, 0, 2, 0, 3, 0, 1, 17, 0};
    suffix_array<int> sa(a.begin(), a.end());
  }
  {
    std::vector<size_t> a{0, 1, 17, 0, 2, 0, 3, 0, 1, 17, 0};
    // std::vector<size_t> a{1, 2, 18, 1, 3, 1, 4, 1, 2, 18, 1};
    suffix_array<size_t> sa(a.begin(), a.end());
  }
  {
    std::string a = "abracadabra";
    suffix_array<char> sa(a.begin(), a.end());
  }
  {
    std::string a = "a";
    suffix_array<char>(a.begin(), a.end());
    suffix_array<char>(a.begin(), a.begin());
  }
}
