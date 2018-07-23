#ifndef _FINITE_ELEMENT_METHOD__FEM_TYPES_H_
#define _FINITE_ELEMENT_METHOD__FEM_TYPES_H_

#include <iostream>

#include <cmath>


#include "NumericFunctions.h"

namespace FiniteElementMethod {


  class ThreeVector {

  public :

     static const int BlockSize = 3;

     typedef int IndexType;


     ThreeVector () {}
     ThreeVector (const double array[BlockSize]);
     ~ThreeVector () {}

     double  operator ()( const int Position ) const;
     double& operator ()( const int Position );

     int dimension () const;

     void zero ();

     void fill ( const double withTheValue );

     void put ( const std::string& name,
                std::ostream&      o = std::cout ) const;

     template <typename T>
     ThreeVector& operator= ( const T array[BlockSize] );

     ThreeVector& operator+= ( const ThreeVector& vec );

     ThreeVector& operator-= ( const ThreeVector& vec );

     ThreeVector& operator+= ( const double scalar );

     ThreeVector& operator-= ( const double scalar );

     ThreeVector& operator*= ( const double scalar );

     ThreeVector& operator/= ( const double scalar );

     friend double length ( const ThreeVector& Vec );

     friend void scale (       ThreeVector& Vec,
                         const double   scale );

     friend void normalise ( ThreeVector& vec );

     friend double innerProduct ( const ThreeVector& Left,
                                  const ThreeVector& Right );


  private :

     double m_entries [ BlockSize ];

  };

   double maximum ( const ThreeVector& vec );

   double maximumAbs ( const ThreeVector& vec );

   ThreeVector operator+ ( const ThreeVector& vec1,
                           const ThreeVector& vec2 );

   ThreeVector operator- ( const ThreeVector& vec1,
                           const ThreeVector& vec2 );

   ThreeVector operator* ( const double       scalar,
                           const ThreeVector& vec1 );

   ThreeVector operator* ( const ThreeVector& vec1,
                           const double       scalar );

   ThreeVector operator/ ( const ThreeVector& vec1,
                           const double scalar );

  //------------------------------------------------------------//

  class Matrix3x3 {

  public :

    Matrix3x3 () {}
    ~Matrix3x3 () {}

    double  operator ()( const int row, const int col ) const;
    double& operator ()( const int row, const int col );

    Matrix3x3& operator*= ( const double scalar );

    /// \brief Increment the matrix by matrix value.
    Matrix3x3& operator+= ( const Matrix3x3& mat );

    void zero ();

    void put ( const std::string& name,
               std::ostream&      o = std::cout ) const;

    //------------------------------------------------------------//



  private :

    double m_entries [ 3 ][ 3 ];

  };

   void scale (       Matrix3x3& Mat,
                const double     Factor );

   void matrixVectorProduct ( const Matrix3x3&   Mat,
                              const ThreeVector& Vec,
                                    ThreeVector& Result );

   ThreeVector matrixVectorProduct ( const Matrix3x3&   mat,
                                     const ThreeVector& vec );

   void matrixTransposeVectorProduct ( const Matrix3x3&   Mat,
                                       const ThreeVector& Vec,
                                             ThreeVector& Result );

   void matrixMatrixProduct ( const Matrix3x3& Left,
                              const Matrix3x3& Right,
                                    Matrix3x3& Result );

   void add ( const Matrix3x3& Left,
              const Matrix3x3& Right,
                    Matrix3x3& Result );

   void matrixMatrixTransposeProduct ( const Matrix3x3& Left,
                                       const Matrix3x3& Right,
                                             Matrix3x3& Result );

   void matrixTransposeMatrixProduct ( const Matrix3x3& Left,
                                       const Matrix3x3& Right,
                                             Matrix3x3& Result );


   double determinant ( const Matrix3x3& M );

   void invert ( const Matrix3x3& Mat,
                       Matrix3x3& Inverse );

   void invert ( const Matrix3x3& Mat,
                       Matrix3x3& Inverse,
                       double&    determinant );

  //------------------------------------------------------------//


  class ElementVector {

  public :

     static const unsigned int NumberOfEntries = 8;

    ElementVector () {}
    ~ElementVector () {}

    double  operator ()( const int Position ) const;
    double& operator ()( const int Position );

    int dimension () const;

    void operator=( const ElementVector& Vec );

    void zero ();

    void put ( const std::string& name,
               std::ostream&      o = std::cout ) const;

    void fill ( const double With_The_Value );

    /// Just a temporary function used for testing
    void Randomise ();


     const double* data () const {
        return &m_entries [ 0 ];
     }

     double* data () {
        return &m_entries [ 0 ];
     }

    //----------------------------//

    //----------------------------//

  private :

    double m_entries [ NumberOfEntries ];

  };


   ElementVector maximum ( const ElementVector& vec,
                           const double         scalar );

   ElementVector maximum ( const ElementVector& vec1,
                           const ElementVector& vec2 );

   ElementVector minimum ( const ElementVector& vec1,
                           const ElementVector& vec2 );

   void interpolate ( const ElementVector& start,
                      const ElementVector& end,
                      const double         lambda,
                            ElementVector& result );


   void add ( const ElementVector& Left,
              const ElementVector& Right,
                    ElementVector& Result );

   void add ( const ElementVector& Left,
              const double         Right_Scale,
              const ElementVector& Right,
                    ElementVector& Result );

   void add ( const double         Left_Scale,
              const ElementVector& Left,
              const double         Right_Scale,
              const ElementVector& Right,
                    ElementVector& Result );

   void subtract ( const ElementVector& Left,
                   const ElementVector& Right,
                         ElementVector& Result );


   void Increment ( const ElementVector& Right,
                          ElementVector& Result );

   void Increment ( const double         Right_Scale,
                    const ElementVector& Right,
                          ElementVector& Result );


   void scale (       ElementVector& Result,
                const double         Factor );

   double innerProduct ( const ElementVector& Left,
                         const ElementVector& Right );

   double length ( const ElementVector& Vec );

   double maxValue ( const ElementVector& Vec );


  //------------------------------------------------------------//

   template<class MultiScalar>
   class MultiComponentVector {

   public :

      static const unsigned int NumberOfEntries = 8;

      const MultiScalar& operator ()( const int i ) const;

      MultiScalar& operator ()( const int i );

      MultiScalar dot ( const ElementVector& basis ) const;


   private :

      MultiScalar m_components [ NumberOfEntries ];

   };

  //------------------------------------------------------------//


  class GradElementVector {

  public :

    GradElementVector () {}
    ~GradElementVector () {}

    double  operator ()( const int row, const int col ) const;
    double& operator ()( const int row, const int col );

    int numberOfRows () const;

    int numberOfColumns () const;

    void zero ();

    //----------------------------//

    void put ( const std::string& name,
               std::ostream&      o = std::cout ) const;

    //----------------------------//


  private :

    double m_entries [ 8 ][ 3 ];

  };

    void add ( const GradElementVector& Left,
               const GradElementVector& Right,
                     GradElementVector& Result );

   void add ( const GradElementVector& Left,
              const double             Right_Scale,
              const GradElementVector& Right,
                    GradElementVector& Result );

   void add ( const double             Left_Scale,
              const GradElementVector& Left,
              const double             Right_Scale,
              const GradElementVector& Right,
                    GradElementVector& Result );

   void subtract ( const GradElementVector& Left,
                   const GradElementVector& Right,
                         GradElementVector& Result );

    //----------------------------//

   void matrixVectorProduct ( const GradElementVector& Left,
                              const ThreeVector&       Right,
                                    ElementVector&     Result );

   void matrixTransposeVectorProduct ( const GradElementVector& Left,
                                       const ElementVector&     Right,
                                             ThreeVector&       Result );

   void matrixMatrixProduct ( const GradElementVector& Left,
                              const Matrix3x3&         Right,
                                    GradElementVector& Result );

   void matrixMatrixTransposeProduct ( const GradElementVector& Left,
                                       const Matrix3x3&         Right,
                                             GradElementVector& Result );


  //------------------------------------------------------------//


  class ElementMatrix {

  public :

    ElementMatrix ();
    ~ElementMatrix ();
    /** Copy constructor */
    ElementMatrix(const ElementMatrix& other) = delete;
    /** Copy assignment operator */
    ElementMatrix& operator= (const ElementMatrix& other) = delete;
    /** Move constructor */
    ElementMatrix(ElementMatrix&& other) = delete;
    /** Move assignment operator */
    ElementMatrix& operator= (ElementMatrix&& other) = delete;

    double* C_Array ();

    double  operator ()( const int row, const int col ) const;
    double& operator ()( const int row, const int col );

    int numberOfRows () const;

    int numberOfColumns () const;

    void zero ();

    void put ( const std::string& name,
               std::ostream&      o = std::cout ) const;


    void fill ( const double With_The_Value );
    //
    //
    // Just a temporary function used for testing
    //
    void Randomise ();

    //----------------------------//

    //----------------------------//

  private :

    double  m_allEntries [ 64 ];
    double* m_entries [ 8 ];

  };




   void add ( const ElementMatrix& Left,
              const ElementMatrix& Right,
                    ElementMatrix& Result );

   void add ( const ElementMatrix& Left,
              const double         Right_Scale,
              const ElementMatrix& Right,
                    ElementMatrix& Result );

   void add ( const double         Left_Scale,
              const ElementMatrix& Left,
              const double         Right_Scale,
              const ElementMatrix& Right,
                    ElementMatrix& Result );

   void subtract ( const ElementMatrix& Left,
                   const ElementMatrix& Right,
                         ElementMatrix& Result );


   void scale (       ElementMatrix& Result,
                const double         Factor );

    //----------------------------//

   void OuterProduct ( const ElementVector&      Left,
                       const ElementVector&      Right,
                             ElementMatrix&      Result );

   void addOuterProduct ( const ElementVector&      Left,
                          const ElementVector&      Right,
                                ElementMatrix&      Result );

   void addOuterProduct ( const double              scale,
                          const ElementVector&      Left,
                          const ElementVector&      Right,
                                ElementMatrix&      Result );

   void OuterProduct ( const GradElementVector& Left,
                       const GradElementVector& Right,
                             ElementMatrix&     Result );

   void addOuterProduct ( const double             scale,
                          const GradElementVector& Left,
                          const GradElementVector& Right,
                                ElementMatrix&     Result );

   void addOuterProduct ( const GradElementVector& Left,
                          const GradElementVector& Right,
                                ElementMatrix&     Result );


  //------------------------------------------------------------//


  class ElementGeometryMatrix {

  public :

    ElementGeometryMatrix ();
    ~ElementGeometryMatrix () {}
    /** Copy constructor */
    ElementGeometryMatrix(const ElementGeometryMatrix& other) = delete;
    /** Copy assignment operator */
    ElementGeometryMatrix& operator= (const ElementGeometryMatrix& other) = delete;
    /** Move constructor */
    ElementGeometryMatrix(ElementGeometryMatrix&& other) = delete;
    /** Move assignment operator */
    ElementGeometryMatrix& operator= (ElementGeometryMatrix&& other) = delete;

    double  operator ()( const int row, const int col ) const;
    double& operator ()( const int row, const int col );

    int numberOfRows () const;

    int numberOfColumns () const;

    void zero ();

    void put ( const std::string& name,
               std::ostream&      o = std::cout ) const;

     double getMidPoint ();

  private :

    double m_allEntries [ 24 ];
    double* m_entries [ 3 ];

  };

    // friend std::ostream& operator<< ( std::ostream& o, const ElementGeometryMatrix& Mat );

    //----------------------------//


   void matrixMatrixProduct ( const ElementGeometryMatrix& Geometry_Matrix,
                              const GradElementVector&     Grad_Basis,
                                    Matrix3x3&             Jacobian );

  //------------------------------------------------------------//


  class BooleanVector {

  public :

    BooleanVector () {}
    ~BooleanVector () {}

    bool  operator ()( const int Position ) const;
    bool& operator ()( const int Position );

    bool orValues () const;

    int dimension () const {
      return 8;
    }

    void put () const;

    void fill ( const bool With_The_Value );

  private :

    bool m_entries [ 8 ];

  };


  //------------------------------------------------------------//

  ///
  /// ThreeVector inline functions
  ///

  inline ThreeVector::ThreeVector (const double array[3])
  {
     * this = array;
  }

  template <typename T>
  ThreeVector& ThreeVector::operator= ( const T array[3] )
  {
     m_entries[0] = (double) array[0];
     m_entries[1] = (double) array[1];
     m_entries[2] = (double) array[2];

     return * this;
  }

  inline double ThreeVector::operator ()( const int Position ) const {

    #ifdef _Constraint_Check_
    if ( Position < 1 || Position > 3 ) {
      cout << " Position (" << Position << ") not in range 1 .. 3" << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }

  //------------------------------------------------------------//

  inline double& ThreeVector::operator ()( const int Position ) {
    #ifdef _Constraint_Check_

    if ( Position < 1 || Position > 3 ) {
      cout << " Position (" << Position << ") not in range 1 .. 3" << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }

  //------------------------------------------------------------//

  inline int ThreeVector::dimension () const {
    return 3;
  }

  //------------------------------------------------------------//

  inline void ThreeVector::zero () {
     fill ( 0.0 );
  }

  //------------------------------------------------------------//

  inline void ThreeVector::fill ( const double withTheValue ) {
    m_entries [ 0 ] = withTheValue;
    m_entries [ 1 ] = withTheValue;
    m_entries [ 2 ] = withTheValue;
  }

  //------------------------------------------------------------//

  inline double length ( const ThreeVector& Vec ) {

    double Result;

    Result = sqrt ( Vec ( 1 ) * Vec ( 1 ) + Vec ( 2 ) * Vec ( 2 ) + Vec ( 3 ) * Vec ( 3 ));

    return Result;
  }

  //------------------------------------------------------------//

  inline void scale (       ThreeVector& Vec,
                      const double        scale ) {

    Vec ( 1 ) = scale * Vec ( 1 );
    Vec ( 2 ) = scale * Vec ( 2 );
    Vec ( 3 ) = scale * Vec ( 3 );

  }

  //------------------------------------------------------------//

  inline double innerProduct ( const ThreeVector& Left,
                               const ThreeVector& Right ) {
    return Left ( 1 ) * Right ( 1 ) + Left ( 2 ) * Right ( 2 ) + Left( 3 ) * Right( 3 );
  }

  //------------------------------------------------------------//

  inline double innerProduct ( const ElementVector& Left,
                               const ElementVector& Right ) {
    return Left ( 1 ) * Right ( 1 ) + Left ( 2 ) * Right ( 2 ) + Left( 3 ) * Right( 3 ) + Left( 4 ) * Right( 4 ) +
           Left ( 5 ) * Right ( 5 ) + Left ( 6 ) * Right ( 6 ) + Left( 7 ) * Right( 7 ) + Left( 8 ) * Right( 8 );
  }

  //------------------------------------------------------------//

  inline void normalise ( ThreeVector& vec ) {
     FiniteElementMethod::scale ( vec, 1.0 / FiniteElementMethod::length ( vec ));
  }


  inline ThreeVector operator+ ( const ThreeVector& vec1,
                                 const ThreeVector& vec2 ) {
     ThreeVector result = vec1;
     result += vec2;
     return result;
  }

  inline ThreeVector operator- ( const ThreeVector& vec1,
                                 const ThreeVector& vec2 ) {
     ThreeVector result = vec1;
     result -= vec2;
     return result;
  }


   inline ThreeVector operator* ( const double       scalar,
                                  const ThreeVector& vec1 ) {
      return vec1 * scalar;
  }

  inline ThreeVector operator* ( const ThreeVector& vec1,
                                 const double       scalar ) {
     ThreeVector result = vec1;
     result *= scalar;
     return result;
  }

  inline ThreeVector operator/ ( const ThreeVector& vec1,
                                 const double scalar ) {
     ThreeVector result = vec1;
     result /= scalar;
     return result;
  }

  inline ThreeVector& ThreeVector::operator+= ( const double scalar ) {
     m_entries [ 0 ] += scalar;
     m_entries [ 1 ] += scalar;
     m_entries [ 2 ] += scalar;
     return *this;
  }

  inline ThreeVector& ThreeVector::operator-= ( const double scalar ) {
     m_entries [ 0 ] -= scalar;
     m_entries [ 1 ] -= scalar;
     m_entries [ 2 ] -= scalar;
     return *this;
  }

  inline ThreeVector& ThreeVector::operator/= ( const double scalar ) {
     m_entries [ 0 ] /= scalar;
     m_entries [ 1 ] /= scalar;
     m_entries [ 2 ] /= scalar;
     return *this;
  }

  inline ThreeVector& ThreeVector::operator*= ( const double scalar ) {
     m_entries [ 0 ] *= scalar;
     m_entries [ 1 ] *= scalar;
     m_entries [ 2 ] *= scalar;
     return *this;
  }


  inline ThreeVector& ThreeVector::operator+= ( const ThreeVector& vec ) {
     m_entries [ 0 ] += vec.m_entries [ 0 ];
     m_entries [ 1 ] += vec.m_entries [ 1 ];
     m_entries [ 2 ] += vec.m_entries [ 2 ];
     return *this;
  }

  inline ThreeVector& ThreeVector::operator-= ( const ThreeVector& vec ) {
     m_entries [ 0 ] -= vec.m_entries [ 0 ];
     m_entries [ 1 ] -= vec.m_entries [ 1 ];
     m_entries [ 2 ] -= vec.m_entries [ 2 ];
     return *this;
  }

   inline double maximum ( const ThreeVector& vec ) {
      return NumericFunctions::Maximum3 ( vec ( 1 ), vec ( 2 ), vec ( 3 ));
   }

   inline double maximumAbs ( const ThreeVector& vec ) {
      return NumericFunctions::Maximum3 ( std::abs ( vec ( 1 )), std::abs ( vec ( 2 )), std::abs ( vec ( 3 )));
   }

  //------------------------------------------------------------//

  ///
  /// Matrix 3x3 inline functions
  ///

  inline double Matrix3x3::operator ()( const int row, const int col ) const {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 3 || Col < 1 || Col > 3 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 3" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline double& Matrix3x3::operator ()( const int row, const int col ) {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 3 || Col < 1 || Col > 3 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 3" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline void Matrix3x3::zero () {
    m_entries [ 0 ][ 0 ] = 0.0; m_entries [ 0 ][ 1 ] = 0.0; m_entries [ 0 ][ 2 ] = 0.0;
    m_entries [ 1 ][ 0 ] = 0.0; m_entries [ 1 ][ 1 ] = 0.0; m_entries [ 1 ][ 2 ] = 0.0;
    m_entries [ 2 ][ 0 ] = 0.0; m_entries [ 2 ][ 1 ] = 0.0; m_entries [ 2 ][ 2 ] = 0.0;
  }


  //------------------------------------------------------------//

  inline void scale (       Matrix3x3& Mat,
                      const double      Factor ) {

    Mat ( 1, 1 ) *= Factor;      Mat ( 1, 2 ) *= Factor;      Mat ( 1, 3 ) *= Factor;
    Mat ( 2, 1 ) *= Factor;      Mat ( 2, 2 ) *= Factor;      Mat ( 2, 3 ) *= Factor;
    Mat ( 3, 1 ) *= Factor;      Mat ( 3, 2 ) *= Factor;      Mat ( 3, 3 ) *= Factor;

  }

   inline Matrix3x3& Matrix3x3::operator+= ( const Matrix3x3& mat ) {

      m_entries [ 0 ][ 0 ] += mat.m_entries [ 0 ][ 0 ]; m_entries [ 0 ][ 1 ] += mat.m_entries [ 0 ][ 1 ]; m_entries [ 0 ][ 2 ] += mat.m_entries [ 0 ][ 2 ];
      m_entries [ 1 ][ 0 ] += mat.m_entries [ 1 ][ 0 ]; m_entries [ 1 ][ 1 ] += mat.m_entries [ 1 ][ 1 ]; m_entries [ 1 ][ 2 ] += mat.m_entries [ 1 ][ 2 ];
      m_entries [ 2 ][ 0 ] += mat.m_entries [ 2 ][ 0 ]; m_entries [ 2 ][ 1 ] += mat.m_entries [ 2 ][ 1 ]; m_entries [ 2 ][ 2 ] += mat.m_entries [ 2 ][ 2 ];

      return *this;
   }

   inline void add ( const Matrix3x3& left,
                     const Matrix3x3& right,
                           Matrix3x3& result ) {
      result ( 1, 1 ) = left ( 1, 1 ) + right ( 1, 1 ); result ( 1, 2 ) = left ( 1, 2 ) + right ( 1, 2 ); result ( 1, 3 ) = left ( 1, 3 ) + right ( 1, 3 );
      result ( 2, 1 ) = left ( 2, 1 ) + right ( 2, 1 ); result ( 2, 2 ) = left ( 2, 2 ) + right ( 2, 2 ); result ( 2, 3 ) = left ( 2, 3 ) + right ( 2, 3 );
      result ( 3, 1 ) = left ( 3, 1 ) + right ( 3, 1 ); result ( 3, 2 ) = left ( 3, 2 ) + right ( 3, 2 ); result ( 3, 3 ) = left ( 3, 3 ) + right ( 3, 3 );
   }


  //------------------------------------------------------------//

  ///
  /// ElementVector inline functions
  ///

  inline double ElementVector::operator ()( const int Position ) const {

    #ifdef _Constraint_Check_
    if ( Position < 1 ||Position  > NumberOfEntries ) {
       cout << " Position (" << Position << ") not in range 1 .. " << NumberOfEntries << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }

  //------------------------------------------------------------//

  inline double& ElementVector::operator ()( const int Position ) {

    #ifdef _Constraint_Check_
    if ( Position < 1 || Position > NumberOfEntries ) {
       cout << " Position (" << Position << ") not in range 1 .. " << NumberOfEntries << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }

  //------------------------------------------------------------//

  inline int ElementVector::dimension () const {
    return NumberOfEntries;
  }

  //------------------------------------------------------------//

  ///
  /// GradElementVector inline functions
  ///

  inline double GradElementVector::operator ()( const int row, const int col ) const {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 8 || Col < 1 || Col > 3 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 8, 1 .. 3" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline double& GradElementVector::operator ()( const int row, const int col ) {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 8 || Col < 1 || Col > 3 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 8, 1 .. 3" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline int GradElementVector::numberOfRows () const {
    return 8;
  }

  //------------------------------------------------------------//

  inline int GradElementVector::numberOfColumns () const {
    return 3;
  }

  //------------------------------------------------------------//

  ///
  /// ElementMatrix inline functions
  ///

  inline double* ElementMatrix::C_Array () {
    return &m_allEntries [ 0 ];
  }

  //------------------------------------------------------------//

  inline double ElementMatrix::operator ()( const int row, const int col ) const {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 8 || Col < 1 || Col > 8 ) {
      cout << " Element Row or Col (" << Row << ", " << Col << ") not in range 1 .. 8, 1 .. 8" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline double& ElementMatrix::operator ()( const int row, const int col ) {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 8 || Col < 1 || Col > 8 ) {
      cout << " Assign Row or Col (" << Row << ", " << Col << ") not in range 1 .. 8, 1 .. 8" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //------------------------------------------------------------//

  inline int ElementMatrix::numberOfRows () const {
    return 8;
  }

  //------------------------------------------------------------//

  inline int ElementMatrix::numberOfColumns () const {
    return 8;
  }

  //-------------------------------------------------------------//

  ///
  /// ElementGeometryMatrix inline functions
  ///

  inline double ElementGeometryMatrix::operator ()( const int row, const int col ) const {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 3 || Col < 1 || Col > 8 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 3, 1 .. 8" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //-------------------------------------------------------------//

  inline double& ElementGeometryMatrix::operator ()( const int row, const int col ) {

    #ifdef _Constraint_Check_
    if ( Row < 1 || Row > 3 || Col < 1 || Col > 8 ) {
      cout << " Row or Col (" << Row << ", " << Col << ") not in range 1 .. 3, 1 .. 8" << endl;
    }
    #endif

    return m_entries [ row - 1 ][ col - 1 ];
  }

  //-------------------------------------------------------------//

  inline int ElementGeometryMatrix::numberOfRows () const {
    return 3;
  }

  //-------------------------------------------------------------//

  inline int ElementGeometryMatrix::numberOfColumns () const {
    return 8;
  }

  //------------------------------------------------------------//

  ///
  /// Boolean_Array inline functions
  ///

  inline bool BooleanVector::operator ()( const int Position ) const {

    #ifdef _Constraint_Check_
    if ( Position < 1 || Position > 8 ) {
      cout << " Position (" << Position << ") not in range 1 .. 8" << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }

  inline bool& BooleanVector::operator ()( const int Position ) {

    #ifdef _Constraint_Check_
    if ( Position < 1 || Position > 8 ) {
      cout << " Position (" << Position << ") not in range 1 .. 8" << endl;
    }
    #endif

    return m_entries [ Position - 1 ];
  }


   template<class MultiScalar>
   inline const MultiScalar& MultiComponentVector<MultiScalar>::operator ()( const int i ) const {
      return m_components [ i - 1 ];
   }

   template<class MultiScalar>
   inline MultiScalar& MultiComponentVector<MultiScalar>::operator ()( const int i ) {
      return m_components [ i - 1 ];
   }


   template<class MultiScalar>
   MultiScalar MultiComponentVector<MultiScalar>::dot ( const ElementVector& basis ) const {

      typedef typename MultiScalar::IndexType IndexType;

      MultiScalar result;
      IndexType index;
      int i;

      for ( i = 0; i < MultiScalar::BlockSize; ++i ) {
         index = IndexType ( i );
         result ( index ) = basis ( 1 ) * m_components [ 0 ]( index ) +
                            basis ( 2 ) * m_components [ 1 ]( index ) +
                            basis ( 3 ) * m_components [ 2 ]( index ) +
                            basis ( 4 ) * m_components [ 3 ]( index ) +
                            basis ( 5 ) * m_components [ 4 ]( index ) +
                            basis ( 6 ) * m_components [ 5 ]( index ) +
                            basis ( 7 ) * m_components [ 6 ]( index ) +
                            basis ( 8 ) * m_components [ 7 ]( index );
      }

      return result;
   }


} // end namespace FiniteElementMethod

#endif // _FINITE_ELEMENT_METHOD__FEM_TYPES_H_
