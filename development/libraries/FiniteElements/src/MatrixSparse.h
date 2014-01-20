#ifndef _MATRIX_SPARSE_H_
#define _MATRIX_SPARSE_H_

#include <eigen3/Eigen/SparseCore>


class MatrixSparse : public Eigen::SparseMatrix<double, Eigen::ColMajor> {
   
public:
   MatrixSparse( const int NumberOfPoint, const int BlockSize );
   
   double & operator ()( const int row, const int col );
   
};

inline double& MatrixSparse::operator () ( const int row, const int col ) {
   return insert( row,  col );
}

inline MatrixSparse::MatrixSparse ( const int NumberOfPoints, const int BlockSize ) {
   resize( NumberOfPoints * 3, NumberOfPoints * BlockSize );
   reserve( NumberOfPoints * 3 * BlockSize );
   setZero();
}

#endif
