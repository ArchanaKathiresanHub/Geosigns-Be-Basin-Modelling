#ifndef __Numerics_Vector_HH__
#define __Numerics_Vector_HH__

#include <iostream>

#include "Numerics.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file Vector.h
  /// \brief Provides the vector class.

  /// \brief Base vector class used throught the code.
  ///
  /// Entries are accessed from the half-open set of [0,dimension)
  class Vector {

  public :

    /// \name Constructors/destructors
    /// @{

    /// \brief Standard constructor
    Vector ();

    /// \brief Standard constructor
    Vector ( const int size );

    /// \brief Copy constructor
    Vector ( const Vector& vec );

    virtual ~Vector ();

    /// @}

    /// \name Post construction constructor
    /// @{

    /// \brief Set the vector to a new size.
    void resize ( const int newSize );

    /// \brief Fills the vector with a specified value.
    void fill ( const FloatingPoint withTheValue );

    /// @}
    /// \name Accessor operations
    /// @{

    /// \brief Accessor function, returns entry at position subscript.
    FloatingPoint  operator ()( const int subscript ) const;

    /// \brief Accessor function, returns entry at position subscript.
    FloatingPoint& operator ()( const int subscript );

    /// \brief Returns the contiguous array containing the vector values.
    FloatingPoint const* data () const;

    /// \brief Returns the contiguous array containing the vector values.
    FloatingPoint* data ();

    /// \brief Returns the dimension of the vector.
    int dimension () const;

    /// @}

    /// \name Arithmetic operations.
    /// @{

    /// \brief Add vector vec to the current vector.
    Vector& operator += ( const Vector& vec );

    /// \brief Subtract vector vec from the current vector.
    Vector& operator -= ( const Vector& vec );

    /// \brief Scale vector by some scalar value. 
    Vector& operator *= ( const FloatingPoint scalar );

    /// \brief Assignment operator.
    Vector& operator=( const Vector& vec );

    /// @}

    /// \name Miscellaneous operations.
    /// @{

    /// \brief Print values of the vector to the output stream in Matlab ASCII format.
    void print ( const std::string&  vectorName,
                       std::ostream& output ) const;

    /// @}

  protected :

    /// \brief Copy contents of vector to this vector.
    void copy ( const Vector& vec );

    /// \brief The low level array holding the values of the vector.
    FloatingPoint* values;

    /// \brief The number of entries that are stored in the current vector.
    int vectorDimension;

  };


  typedef std::vector <Vector*> VectorArray;

  /// \name Vector operations.
  ///
  /// Where possible, these functions will call the BLAS library functions.
  /// @{

  /// \brief Inner product of two vectors, computes \f$ \sum u_i \cdot v_i \f$.
  FloatingPoint innerProduct ( const Vector& u,
                               const Vector& v );

  /// \brief 2 norm of vector, computes  \f$ \sqrt {\left( \sum v_i \cdot v_i \right)} \f$.
  FloatingPoint vectorLength ( const Vector& v );

  /// \brief Determine if vector contains all zero values.
  bool isZeroVector ( const Vector& v );

  /// \brief Compute \f$ y = a \cdot x + y \f$.
  void vectorAXPY ( const FloatingPoint A,
                    const Vector&       X,
                          Vector&       Y );

  /// \brief Compute \f$ y = x - y \f$.
  void vectorXMY ( const Vector&        X,
                         Vector&        Y );

  /// \brief Compute \f$ y = y - x \f$.
  void vectorYMX ( const Vector&        X,
                         Vector&        Y );

  /// \brief Compute \f$ z = x - y \f$. 
  void vectorXMY ( const Vector&        X,
                   const Vector&        Y,
                         Vector&        Z );

  /// \brief Compute \f$ x = a \cdot x \f$. 
  void vectorScale ( const FloatingPoint a,
                           Vector&       x );

  /// \brief Compute \f$ y = a \cdot x \f$. 
  void vectorScale ( const FloatingPoint a,
                     const Vector&       x,
                           Vector&       y );

  /// @}

  /** @} */

}

//------------------------------------------------------------//

//
// Inline functions
//

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::Vector::operator ()( const int subscript ) const {

//   if ( 0 > subscript || subscript >= vectorDimension ) {
//   }

  return values [ subscript ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint& Numerics::Vector::operator ()( const int subscript ) {

//   if ( subscript < 0 || subscript >= vectorDimension ) {
//     std::cout << " error 2 " << subscript << " not in 0 .. " << vectorDimension << std::endl;
//   }

  return values [ subscript ];
}

//------------------------------------------------------------//

inline int Numerics::Vector::dimension () const {
  return vectorDimension;
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint const* Numerics::Vector::data () const {
  return values;
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint* Numerics::Vector::data () {
  return values;
}

//------------------------------------------------------------//


#endif // __Numerics_Vector_HH__
