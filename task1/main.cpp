#include "matrix_mult.hpp"

#include <iostream>
#include <vector>

int main() {
  const std::vector<std::vector<int>> matrix1 = {
    {1, 2, 3},
    {4, 5, 6}
  };
  const std::vector<std::vector<int>> matrix2 = {
    {1, 2},
    {3, 4},
    {5, 6}
  };

  const auto mult_result = matrix_mult::Multiply(matrix1, matrix2, 2);

  for (const auto& line : mult_result) {
    for (auto item : line) {
      std::cout << item << ' ';
    }
    std::cout << '\n';
  }

  return 0;
}