#ifndef _MATRIX_8x8_H_
#define _MATRIX_8x8_H_

#include <eigen3/Eigen/Core>


typedef Eigen::Matrix<double, 8, 8, Eigen::RowMajor> Matrix8x8;

namespace FiniteElementMethod {

void scale ( Matrix8x8& Mat, const double Factor );

}

#endif
