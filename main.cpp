#include "src.hpp"
#include <iostream>

// Standalone runner: each input line is a pattern and a nonempty candidate.
int main() {
  std::string pattern, candidate;
  while (std::cin >> pattern >> candidate)
    std::cout << Grammar::RegexChecker(pattern).Check(candidate) << '\n';
}
