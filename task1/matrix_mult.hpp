#pragma once

#include <vector>

namespace matrix_mult {

using LineMatrix = std::vector<int>;
using SquareMatrix = std::vector<std::vector<int>>;

SquareMatrix Multiply(const SquareMatrix& matrix1, const SquareMatrix& matrix2, size_t thread_num);

}  // namespace matrix_mult


