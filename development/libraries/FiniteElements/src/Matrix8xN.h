#ifndef _MATRIX_8xN_H_
#define _MATRIX_8xN_H_

#include <eigen3/Eigen/Core>


class Matrix8xN : public Eigen::Matrix<double, 8, Eigen::Dynamic, Eigen::RowMajor> {
   
public:
   Matrix8xN( const int NumberOfCols ) {
      resize( Eigen::NoChange, NumberOfCols );
      setZero();
   } 
};


#endif
