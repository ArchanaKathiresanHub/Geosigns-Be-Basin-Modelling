#ifndef __NUMERICS__POINT_H__
#define __NUMERICS__POINT_H__

#include "Numerics.h"
#include "GeometryVector.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>

namespace Numerics {

   /** @addtogroup Numerics
    *
    * @{
    */

   /// \file Point.h
   /// \brief Adds a 3-d point and an array of this type to the Numerics namespace.

   /// \brief A point in 3D space.
   class Point {

   public :

      /// \brief The dimension of the domain.
      enum { DIMENSION = 3 };

      Point ();

      Point ( const Point& p );

      Point ( const FloatingPoint initialX,
              const FloatingPoint initialY,
              const FloatingPoint initialZ );

      /// \name Accessor methods
      /// @{

      /// \brief Return the value at the position in the point.
      FloatingPoint  operator ()( const int coord ) const;

      /// \brief Return a reference to the value at the position in the point.
      FloatingPoint& operator ()( const int coord );

      /// Return the x co-ordinate.
      FloatingPoint x () const;

      /// Return the y co-ordinate.
      FloatingPoint y () const;

      /// Return the z co-ordinate.
      FloatingPoint z () const;

      /// @}

      /// Set all values to zero.
      void zero ();

      /// \name Point arithmetic operations
      /// @{

      /// \brief Displace point position by vector, add vector to point.
      Point& operator+= ( const GeometryVector& v );

      /// \brief Displace point position by vector, subtract vector to point.
      Point& operator-= ( const GeometryVector& v );

      /// \brief Scale each co-ordinate in the point by the a value in the equivalent position in the vector.
      // Should have some kind of affine transformation class to perform this operation.
      Point& operator*= ( const GeometryVector& v );

      /// \brief Scale the point by a scalar value.
      Point& operator*= ( const FloatingPoint scalar );


      /// \brief Displacement vector determining how to get from p1 to p2.
      friend GeometryVector operator-( const Point& p1,
                                       const Point& p2 );

      /// \brief Displace p by adding displacement vector.
      friend Point operator+( const Point&          p,
                              const GeometryVector& v );

      /// \brief Displace p by subtracting displacement vector.
      friend Point operator-( const Point&          p,
                              const GeometryVector& v );

      /// \brief Scale each co-ordinate in the point by the a value in the equivalent position in the vector.
      friend Point operator*( const GeometryVector& v,
                              const Point&          p );

      /// @}

      /// Return a string representation of the point.
      std::string image () const;


   private :

      /// \brief Contains the values of the point.
      FloatingPoint values [ DIMENSION ];

   };

   /// \brief Displacement vector determining how to get from p1 to p2.
   GeometryVector operator-( const Point& p1,
    const Point& p2 );

   /// \brief Displace p by adding displacement vector.
   Point operator+( const Point&          p,
    const GeometryVector& v );

   /// \brief Displace p by subtracting displacement vector.
   Point operator-( const Point&          p,
    const GeometryVector& v );

   /// \brief Scale each co-ordinate in the point by the a value in the equivalent position in the vector.
   Point operator*( const GeometryVector& v,
    const Point&          p );


   /// \brief The square of the distance between 2 points.
   ///
   /// Returns \f$ \sum (p1_i - p2_i)^2 \f$
   /// Often, it will be sufficient to use just the square of the distance
   /// rather than the actual distance, which requires an additional 
   /// square-root evaluation.
   FloatingPoint separationDistanceSquared ( const Point& p1,
                                             const Point& p2 );


   /// \brief The distance between 2 points.
   ///
   /// Returns \f$ \sqrt {\left( \sum (p1_i - p2_i)^2 \right) } \f$
   FloatingPoint separationDistance ( const Point& p1,
                                      const Point& p2 );
   

   /// \typedef PointArray
   /// \brief An array of type Point.
   typedef std::vector<Point> PointArray;

   std::ostream& operator<< ( std::ostream& o, const PointArray& array );

   /** @} */

}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::Point::operator ()( const int coord ) const {
#ifndef NDEBUG
   if( coord < 0 || coord >= DIMENSION ) throw std::runtime_error("Out of bounds access");
#endif
   return values [ coord ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint& Numerics::Point::operator ()( const int coord ) {
#ifndef NDEBUG
   if( coord < 0 || coord >= DIMENSION ) throw std::runtime_error("Out of bounds access");
#endif
   return values [ coord ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::Point::x () const {
   return operator ()( 0 );
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::Point::y () const {
   return operator ()( 1 );
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::Point::z () const {
   return operator ()( 2 );
}
//------------------------------------------------------------//

inline Numerics::Point& Numerics::Point::operator+= ( const GeometryVector& v ) {

   values [ 0 ] += v ( 0 );
   values [ 1 ] += v ( 1 );
   values [ 2 ] += v ( 2 );

   return *this;
}

//------------------------------------------------------------//

inline Numerics::Point& Numerics::Point::operator-= ( const GeometryVector& v ) {

   values [ 0 ] -= v ( 0 );
   values [ 1 ] -= v ( 1 );
   values [ 2 ] -= v ( 2 );

   return *this;
}

//------------------------------------------------------------//

inline Numerics::Point& Numerics::Point::operator*= ( const GeometryVector& v ) {

   values [ 0 ] *= v ( 0 );
   values [ 1 ] *= v ( 1 );
   values [ 2 ] *= v ( 2 );

   return *this;
}

//------------------------------------------------------------//

inline Numerics::Point& Numerics::Point::operator*= ( const FloatingPoint scalar ) {

   values [ 0 ] *= scalar;
   values [ 1 ] *= scalar;
   values [ 2 ] *= scalar;

   return *this;
}

//------------------------------------------------------------//

inline Numerics::GeometryVector Numerics::operator-( const Point& p1,
                                                     const Point& p2 ) {

   GeometryVector result;

   result ( 0 ) = p1 ( 0 ) - p2 ( 0 );
   result ( 1 ) = p1 ( 1 ) - p2 ( 1 );
   result ( 2 ) = p1 ( 2 ) - p2 ( 2 );

   return result;
}

//------------------------------------------------------------//

inline Numerics::Point Numerics::operator+( const Point&          p,
                                            const GeometryVector& v ) {

   Point result;

   result ( 0 ) = p ( 0 ) + v ( 0 );
   result ( 1 ) = p ( 1 ) + v ( 1 );
   result ( 2 ) = p ( 2 ) + v ( 2 );

   return result;

}

//------------------------------------------------------------//

inline Numerics::Point Numerics::operator-( const Point&          p,
                                            const GeometryVector& v ) {

   Point result;

   result ( 0 ) = p ( 0 ) - v ( 0 );
   result ( 1 ) = p ( 1 ) - v ( 1 );
   result ( 2 ) = p ( 2 ) - v ( 2 );

   return result;

}

//------------------------------------------------------------//

inline Numerics::Point Numerics::operator*( const GeometryVector& v,
                                            const Point&          p ) {

   Point result;

   result ( 0 ) = p ( 0 ) * v ( 0 );
   result ( 1 ) = p ( 1 ) * v ( 1 );
   result ( 2 ) = p ( 2 ) * v ( 2 );

   return result;
}

//------------------------------------------------------------//


#endif // __NUMERICS__POINT_H__
