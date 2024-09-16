#include <cstdlib> // Для rand() и srand()
#include <ctime>   // Для time()
#include <fstream>
#include <iostream>
#include <vector>

#include "matrix_mult.hpp"

// Функция для создания матрицы с случайными целыми значениями
std::vector<std::vector<int>> create_random_matrix(size_t height, size_t width) {
    // Инициализация генератора случайных чисел
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Создание матрицы
    std::vector<std::vector<int>> matrix(height, std::vector<int>(width));

    // Заполнение матрицы случайными значениями в диапазоне [0, 9]
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            matrix[i][j] = rand() % 10; // Случайное число от 0 до 9
        }
    }

    return matrix;
}

// Функция для вывода матрицы на экран
void print_matrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
  if (argc != 5) {
    std::cout << "Not enought args!\n";
    return 1;
  }

  std::string filename = "result.txt";
  std::ofstream outFile;
  outFile.open(filename, std::ios::app);
  if (!outFile.is_open()) {
    std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
    return 1;
  }

  size_t h1 = std::atoi(argv[1]);
  size_t w1 = std::atoi(argv[2]);
  size_t w2 = std::atoi(argv[3]);
  size_t threads_num = std::atoi(argv[4]);

  const std::vector<std::vector<int>> matrix1 = create_random_matrix(h1, w1);
  const std::vector<std::vector<int>> matrix2 = create_random_matrix(w1, w2);
  
  const auto mult_result = matrix_mult::Multiply(matrix1, matrix2, threads_num);

  // std::cout << "Matrix1:\n";
  // print_matrix(matrix1);
  // std::cout << "Matrix2:\n";
  // print_matrix(matrix2);

  // std::cout << "MatrixRes:\n";
  // print_matrix(mult_result.first);

  std::cout << "time: " << mult_result.second << "ms";

  outFile << mult_result.second << std::endl;
  outFile.close();

  return 0;
}