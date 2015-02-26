#ifndef LINEAR_ALGEBRA__ALIGNED_DENSE_MATRIX__H
#define LINEAR_ALGEBRA__ALIGNED_DENSE_MATRIX__H

#include <iostream>

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */

   /// \brief Used to indicate if the matrix transpose is required or not.
   enum MatrixTransposeType { NO_TRANSPOSE, TRANSPOSE };

   /// \brief Simple dense matrix structure with storage in column major order.
   ///
   /// All values are contiguous in memory and each column will be aligned 
   /// with with correct double or simd data-type depending on the alignment required.
   class AlignedDenseMatrix {

   public :

      /// \brief Construct matrix with particular size.
      ///
      /// \param [in] m Number of rows.
      /// \param [in] n Number of columns.
      /// \pre m > 0
      /// \pre n > 0
      AlignedDenseMatrix ( const int m, const int n );

      /// \brief Matix copy constructor.
      AlignedDenseMatrix ( const AlignedDenseMatrix& mat );

      /// \brief Destructor.
      ~AlignedDenseMatrix ();

      /// \brief Copy matrix.
      AlignedDenseMatrix& operator=( const AlignedDenseMatrix& mat );


      /// \brief Access value at position.
      ///
      /// \param [in] i Row position.
      /// \param [in] j Column position.
      /// \pre 0 <= i < rows ().
      /// \pre 0 <= j < cols ().
      double operator ()( const int i, const int j ) const;

      /// \brief Access value at position.
      ///
      /// \param [in] i Row position.
      /// \param [in] j Column position.
      /// \pre 0 <= i < rows ().
      /// \pre 0 <= j < cols ().
      double& operator ()( const int i, const int j );

      /// \brief Get the number of rows in the matrix.
      int rows () const;

      /// \brief Get the number of columns in the matrix.
      int cols () const;

      /// \brief Get the leading dimension of the matrix.
      ///
      /// This is the number of doubles stored for each column.
      // Better description of leading dimension.
      int leadingDimension () const;

      /// \brief Fill the matrix with a particular value.
      void fill ( const double withTheValue );

      /// \brief Get access to the low level array of values.
      const double* data () const;

      /// \brief Get access to the low level array of values.
      double* data ();

      /// \brief Get access to the low level array of values for a particular column.
      ///
      /// \param [in] i col position.
      /// \pre 0 <= i < cols ().
      const double* getColumn ( const int col ) const;

      /// \brief Get access to the low level array of values for a particular column.
      ///
      /// \param [in] i col position.
      /// \pre 0 <= i < cols ().
      double* getColumn ( const int col );

      /// \brief Output the matrix to the stream.
      void print ( const std::string& name,
                   std::ostream& o ) const;

   protected :

      /// \brief Get the position in the 1d array of the 2d position requested.
      ///
      /// \param [in] i Row position.
      /// \param [in] j Column position.
      /// \pre 0 <= i < rows ().
      /// \pre 0 <= j < cols ().
      int position ( const int i, const int j ) const;

      /// \brief Allocate the 1d array of values.
      ///
      /// The size of the array allocated will depend on the m_rows and m_cols members.
      void allocate ();

      /// \brief Copy the values of mat to the current object.
      void copy ( const AlignedDenseMatrix& mat );

      /// \brief Resize the current object to match that of the mat parameter.
      void resize ( const AlignedDenseMatrix& mat );

   private :

      /// \brief The 1d values array.
      double* m_values;

      /// \brief The number of rows.
      int m_rows;

      /// \brief The number of columns.
      int m_cols;

      /// \brief The leading dimension.
      int m_leadingDimension;

   };

} // end namespace Numerics

//-----------------
// Inline functions
//-----------------

inline double Numerics::AlignedDenseMatrix::operator ()( const int i, const int j ) const {
   return m_values [ position ( i, j )];
}

inline double& Numerics::AlignedDenseMatrix::operator ()( const int i, const int j ) {
   return m_values [ position ( i, j )];
}

inline int Numerics::AlignedDenseMatrix::rows () const {
   return m_rows;
}

inline int Numerics::AlignedDenseMatrix::cols () const {
   return m_cols;
}

inline int Numerics::AlignedDenseMatrix::leadingDimension () const {
   return m_leadingDimension;
}

inline const double* Numerics::AlignedDenseMatrix::data () const {
   return m_values;
}

inline double* Numerics::AlignedDenseMatrix::data () {
   return m_values;
}

inline const double* Numerics::AlignedDenseMatrix::getColumn ( const int col ) const {
   return &m_values [ col * m_leadingDimension ];
}

inline double* Numerics::AlignedDenseMatrix::getColumn ( const int col ) {
   return &m_values [ col * m_leadingDimension ];
}

inline int Numerics::AlignedDenseMatrix::position ( const int i, const int j ) const {
   return j * m_leadingDimension + i;
}

#endif // LINEAR_ALGEBRA__ALIGNED_DENSE_MATRIX__H
