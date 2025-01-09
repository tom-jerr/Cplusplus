#include "hw1.h"
#include <exception>
#include <random>
#include <stdexcept>
#include <iomanip>
#include <iostream>

namespace algebra {

Matrix zeros(size_t n, size_t m) {
  std::vector<std::vector<double>> tmp(n, std::vector<double>(m, 0));
  return tmp;
}

Matrix ones(size_t n, size_t m) {
  std::vector<std::vector<double>> tmp(n, std::vector<double>(m, 1));
  return tmp;
}

Matrix random(size_t n, size_t m, double min, double max) {
  if(min > max) {
    throw std::logic_error("min must smaller than max");
  }
  std::random_device rd;
  std::mt19937 rd_gen(rd());  // 随机种子生成器
  std::uniform_real_distribution<double> dist(min, max);  // 均匀分布
  std::vector<std::vector<double>> tmp;

  for(size_t j = 0; j < n; ++j) {
    std::vector<double> vec;
    for(size_t i = 0; i < m; ++i) {
      vec.emplace_back(dist(rd_gen));
    }
    tmp.emplace_back(vec);
  }

  return tmp;
}

void show(const Matrix &matrix) {
  std::cout << "| ";
  if (matrix.empty()) {
    std::cout << "|\n";
    return;
  }
  for(const auto& vec : matrix) {
    for(const auto element : vec) {
      std::cout << std::fixed << std::setprecision(3) << element << " ";
    }
    std::cout << "\n";
  }
  std::cout << "|\n";
}

Matrix inverse(const Matrix& matrix) {
  if(matrix.empty()) {
    return Matrix();
  }
  size_t n = matrix.size();
  if (n == 0 || n != matrix[0].size()) throw std::logic_error("Matrix must be non-empty and square.");

  // Create an augmented matrix [A | I]
  Matrix augmented(n, std::vector<double>(2 * n));
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
        augmented[i][j] = matrix[i][j]; // Copy original matrix
        augmented[i][j + n] = (i == j) ? 1.0 : 0.0; // Identity matrix on the right side
    }
  }

  // Apply Gauss-Jordan elimination to transform [A | I] into [I | A^-1]
  for (size_t col = 0; col < n; ++col) {
    // Find pivot in current column
    size_t pivotRow = col;
    for (size_t row = col + 1; row < n; ++row) {
      if (std::abs(augmented[row][col]) > std::abs(augmented[pivotRow][col])) {
        pivotRow = row;
      }
    }
    if (augmented[pivotRow][col] == 0) throw std::logic_error("Matrix is not invertible.");

    // Swap rows to bring pivot to current row
    if (pivotRow != col) {
        augmented = ero_swap(augmented, col, pivotRow);
    }

    // Normalize the pivot row so that the pivot element is 1
    double pivotValue = augmented[col][col];
    augmented = ero_multiply(augmented, col, 1.0 / pivotValue);

    // Eliminate other entries in the current column
    for (size_t row = 0; row < n; ++row) {
      if (row != col && augmented[row][col] != 0) {
        double factor = -augmented[row][col];
        augmented = ero_sum(augmented, col, factor, row);
      }
    }
  }

  // Extract the inverse from the augmented matrix
  Matrix inverse(n, std::vector<double>(n));
  for (size_t i = 0; i < n; ++i) {
      for (size_t j = 0; j < n; ++j) {
          inverse[i][j] = augmented[i][j + n];
      }
  }

  return inverse;
}

Matrix multiply(const Matrix &matrix, double c) {
  if(c == 0) {
    return zeros(matrix.size(), matrix[0].size());
  }
  Matrix tmp(matrix);
  for(size_t i = 0; i < matrix.size(); ++i) {
    for(size_t j = 0; j < matrix[0].size(); ++j) {
      tmp[i][j] *= c;
    }
  }
  return tmp;
}

Matrix multiply(const Matrix &matrix1, const Matrix &matrix2){
  if(matrix1.empty() || matrix2.empty()){
    return Matrix();
  }
  if(matrix1[0].size() != matrix2.size()) {
    throw std::logic_error("matrix1的m需要与matrix2的n相同维度");
  }
  size_t rows1 = matrix1.size();      // Number of rows in matrix1
  size_t cols1 = matrix1[0].size();   // Number of columns in matrix1 (or number of rows in matrix2)
  size_t cols2 = matrix2[0].size();   // Number of columns in matrix2

  // Initialize the result matrix with zeros.
  Matrix tmp(rows1, std::vector<double>(cols2, 0));

  // Perform matrix multiplication.
  for (size_t i = 0; i < rows1; ++i) {
    for (size_t j = 0; j < cols2; ++j) {
      for (size_t k = 0; k < cols1; ++k) {
        tmp[i][j] += matrix1[i][k] * matrix2[k][j];
      }
    }
  }
  return tmp;
}

Matrix sum(const Matrix &matrix, double c){
  if (matrix.empty()) {
    return Matrix();
  }
  Matrix tmp(matrix);
  for(size_t i = 0; i < matrix.size(); ++i) {
    for(size_t j = 0; j < matrix[0].size(); ++j) {
      tmp[i][j] += c;
    }
  }
  return tmp;
}

Matrix sum(const Matrix &matrix1, const Matrix &matrix2){
  if(matrix1.empty() && matrix2.empty()) {
    return Matrix();
  }
  if(matrix1.empty() || matrix2.empty()) {
    throw std::logic_error("must not be empty");
  }
  Matrix tmp(matrix1.size(), std::vector<double>(matrix1[0].size(), 0)); // 初始化tmp

  for(size_t i = 0; i < matrix1.size(); ++i) {
      for(size_t j = 0; j < matrix1[0].size(); ++j) {
          tmp[i][j] = matrix1[i][j] + matrix2[i][j]; // 逐元素相加
      }
  }
  return tmp;
}

Matrix transpose(const Matrix &matrix) {
  if(matrix.empty()) {
    return Matrix();
  }

  Matrix tmp(matrix[0].size(), std::vector<double>(matrix.size(), 0));
  for(size_t i = 0; i < matrix.size(); ++i) {
    for(size_t j = 0; j < matrix[0].size(); ++j) {
      tmp[j][i] = matrix[i][j];
    }
  }
  return tmp;
}

Matrix minor(const Matrix &matrix, size_t n, size_t m) {
    if (matrix.empty() || n >= matrix.size() || m >= matrix[0].size()) {
        throw std::out_of_range("Invalid matrix dimensions or row/column index.");
    }

    // Copy the matrix to avoid modifying the original one.
    Matrix minor(matrix);

    // Remove the specified row.
    if (minor.size() > n) {
        minor.erase(minor.begin() + n);
    }

    // Remove the specified column from each row.
    for (auto& row : minor) {
        if (row.size() > m) {
            row.erase(row.begin() + m);
        }
    }

    return minor;
}

double determinant(const Matrix &matrix){
  size_t n = matrix.size();
  if (n == 0) return 1; // 行列式为1的空矩阵是没有意义的，但这里作为递归终止条件。
  if (n != matrix[0].size()) throw std::invalid_argument("Matrix must be square.");

  // Base case for 2x2 matrix.
  if (n == 2) {
      return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
  }
  double det = 0;
  for (size_t col = 0; col < n; ++col) {
      // Create the minor by removing the first row and the current column.
      Matrix minor2 = minor(matrix, 0, col);

      // Recursively calculate the determinant of the minor.
      det += (col % 2 == 0 ? 1 : -1) * matrix[0][col] * determinant(minor2);
  }
  return det;
}

Matrix concatenate(const Matrix &matrix1, const Matrix &matrix2, int axis) {
  if (axis != 0 && axis != 1) throw std::invalid_argument("axis must be 0 or 1");
  
  Matrix tmp(matrix1);
  if (axis == 0) {
    if(matrix1[0].size() != matrix2[0].size()) {
      throw std::logic_error("must be the same size");
    }
    for(const auto& vec : matrix2)
      tmp.emplace_back(vec);
  } else {
    if(matrix1.size() != matrix2.size()) {
      throw std::logic_error("must be the same size");
    }
    for(size_t i = 0; i < matrix2.size(); ++i) {
      if(i < matrix1.size()) {
        for (size_t j = 0; j < matrix2[0].size(); ++j) {
          tmp[i].emplace_back(matrix2[i][j]);
        }
      } else {
        tmp.emplace_back(matrix2[i]);
      }

    }
  }
  return tmp;
}

Matrix ero_swap(const Matrix &matrix, size_t r1, size_t r2) {
  if (r1 >= matrix.size() || r2 >= matrix.size()) {
      throw std::out_of_range("Row index out of range.");
  }
  Matrix tmp(matrix);
  std::swap(tmp[r1], tmp[r2]);
  return tmp;
}
Matrix ero_multiply(const Matrix &matrix, size_t r, double c) {
  if (r >= matrix.size()) {
    throw std::out_of_range("Row index out of range.");
  }
  Matrix tmp(matrix);
  for (auto& element : tmp[r]) {
    element *= c;
  }
  return tmp;
}
Matrix ero_sum(const Matrix &matrix, size_t r1, double c, size_t r2){
  if (r1 >= matrix.size() || r2 >= matrix.size()) {
    throw std::out_of_range("Row index out of range.");
  }
  Matrix tmp(matrix);
  for (size_t col = 0; col < matrix[r1].size(); ++col) {
    tmp[r2][col] += matrix[r1][col] * c;
  }
  return tmp;
}
Matrix upper_triangular(const Matrix &matrix) {
  if(matrix.empty()) {
    return Matrix();
  }
  size_t rows = matrix.size();
  size_t cols = matrix[0].size();
  if(rows != cols) {
    throw std::logic_error("must be the square matrix");
  }
  Matrix tmp(matrix);
  for (size_t col = 0; col < cols - 1; ++col) { // We don't need to process the last column
    bool foundPivot = false;
    for (size_t row = col; row < rows; ++row) {
      if (tmp[row][col] != 0) {
        // If pivot is not on the diagonal, swap it into place
        if (row != col) {
            tmp = ero_swap(tmp, col, row);
        }
        foundPivot = true;
        break;
      }
    }

    if (!foundPivot) continue; // Skip this column if all elements are zero

    // Eliminate entries below the pivot
    for (size_t row = col + 1; row < rows; ++row) {
      if (tmp[row][col] != 0) {
        double factor = -tmp[row][col] / tmp[col][col];
        tmp = ero_sum(tmp, col, factor, row);
      }
    }
  }

  return tmp;
}
} // namespace algebra