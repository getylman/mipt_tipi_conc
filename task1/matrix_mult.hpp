#pragma once

#include <vector>
#include <utility>

namespace matrix_mult {

using LineMatrix = std::vector<int>;
using SquareMatrix = std::vector<std::vector<int>>;

std::pair<SquareMatrix, double> Multiply(const SquareMatrix& matrix1, const SquareMatrix& matrix2, size_t thread_num);

}  // namespace matrix_mult


