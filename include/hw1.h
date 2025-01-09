#ifndef AP_HW1_H
#define AP_HW1_H
#include <cstddef>
#include <vector>
namespace algebra{
using Matrix = std::vector<std::vector<double>>;
/**
 * @brief 生成n*m的0矩阵
 * 
 * @param n 
 * @param m 
 * @return Matrix 
 */
Matrix zeros(size_t n, size_t m);

/**
 * @brief 生成n*m的1矩阵
 * 
 * @param n 
 * @param m 
 * @return Matrix 
 */
Matrix ones(size_t n, size_t m);

/**
 * @brief 生成大小为 (min, max)大小为n*m的矩阵
 * 
 * @param n 
 * @param m 
 * @param min 
 * @param max 
 * @return Matrix 
 */
Matrix random(size_t n, size_t m, double min, double max);

/**
 * @brief 显示矩阵in a beautiful way
 * 
 * @param matrix 
 */
void show(const Matrix& matrix);
Matrix inverse(const Matrix& matrix);
Matrix multiply(const Matrix& matrix, double c);
Matrix multiply(const Matrix& matrix1, const Matrix& matrix2);
Matrix sum(const Matrix& matrix, double c);
Matrix sum(const Matrix& matrix1, const Matrix& matrix2);
Matrix transpose(const Matrix& matrix);
Matrix minor(const Matrix& matrix, size_t n, size_t m);
double determinant(const Matrix& matrix);
Matrix concatenate(const Matrix& matrix1, const Matrix& matrix2, int axis=0);
Matrix ero_swap(const Matrix& matrix, size_t r1, size_t r2);
Matrix ero_multiply(const Matrix& matrix, size_t r, double c);Matrix ero_sum(const Matrix& matrix, size_t r1, double c, size_t r2);
Matrix upper_triangular(const Matrix& matrix);


} // namespace algebra


#endif //AP_HW1_H
