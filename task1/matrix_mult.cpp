#include "matrix_mult.hpp"

#include <chrono>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

namespace matrix_mult {

namespace {

struct Foo {
    LineMatrix matrix1;
    LineMatrix matrix2;
    size_t height1;
    size_t width1;
    size_t width2;
};

LineMatrix SquareToLine(const SquareMatrix& matrix) {
    LineMatrix result;
    result.reserve(matrix.size() * matrix.front().size());

    for (const auto& line : matrix) {
        for (auto item : line) {
            result.emplace_back(item);
        }
    }

    return result;
}

SquareMatrix LineToSquare(const LineMatrix& matrix, size_t height, size_t width) {
    SquareMatrix result(height, std::vector<int>(width));

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            result[i][j] = matrix[i * width + j];
        }
    }

    return result;
}

LineMatrix TransposeAndToLine(const SquareMatrix& matrix) {
    LineMatrix result(matrix.size() * matrix.front().size());

    size_t height = matrix.size();
    size_t width = matrix.front().size();

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            result[j * height + i] = matrix[i][j];
        }
    }
    
    return result;
}

std::optional<Foo> PreprocessMatrixes(const SquareMatrix& matrix1, const SquareMatrix& matrix2) {
    
    if (matrix1.front().size() != matrix2.size()) {
        return std::nullopt;
    }
    
    Foo result;
    result.matrix1 = SquareToLine(matrix1);
    result.matrix2 = TransposeAndToLine(matrix2);
    result.height1 = matrix1.size();
    result.width1 = matrix2.size();
    result.width2 = matrix2.front().size();

    return result;
}

void ThreadOperation(const Foo& multipliers,
                     SquareMatrix& result,
                     size_t id,
                     size_t threads_num) {
    for (size_t i = 0; i < multipliers.height1; ++i) {
        for (size_t j = id; j < multipliers.width2; j += threads_num) {
            int cur_res = 0;
            for (size_t k = 0; k < multipliers.width1; ++k) {
                cur_res += multipliers.matrix1[i * multipliers.width1 + k] * multipliers.matrix2[j * multipliers.width1 + k];
            }
            result[i][j] = cur_res;
        }
    }
}

}  // namespace

std::pair<SquareMatrix, double> Multiply(const SquareMatrix& matrix1, const SquareMatrix& matrix2, size_t thread_num) {
    const auto multipliers_opt = PreprocessMatrixes(matrix1, matrix2);

    if (!multipliers_opt.has_value()) {
        throw std::invalid_argument("Неверные размеры матрицы или неверный размер вектора.");
    }

    const auto& multipliers = multipliers_opt.value();

    SquareMatrix result(matrix1.size(), std::vector<int>(matrix2.front().size()));

    std::vector<std::thread> threads;

    threads.reserve(thread_num);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < thread_num; ++i) {
        threads.emplace_back(ThreadOperation, std::ref(multipliers), std::ref(result), i, thread_num);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    return {result, elapsed.count()};
}


}  // namespace matrix_mult