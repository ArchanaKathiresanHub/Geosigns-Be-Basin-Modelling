#ifndef _NUMERIC_FUNCTIONS__QUADRATURE_THREE_DIMENSION__H_
#define _NUMERIC_FUNCTIONS__QUADRATURE_THREE_DIMENSION__H_

#include "Quadrature.h"

namespace NumericFunctions {


   class Quadrature3D {

   public :


      /// \brief Iterator over the 3D quadrature rule.
      class Iterator {

      public :

         Iterator ();

         /// Get the x quadrature point.
         double getX () const;

         /// Get the y quadrature point.
         double getY () const;

         /// Get the z quadrature point.
         double getZ () const;

         /// Get the quadrature weight.
         double getWeight () const;

         /// Advance the iterator to the next point.
         Iterator& operator++();

         /// Indicate whenther or not the iteration is finished.
         bool isDone () const;

         /// Initialise the iterator.
         void initialise ();

      private :

         friend class Quadrature3D;

         /// The degree of the quadrature object.
         int                         m_degree   [ 3 ];

         /// The current point of the quadrature object.
         int                         m_position [ 3 ];

         /// All the quadrature points.
         Quadrature::QuadratureArray m_points   [ 3 ];

         /// All the quadrature weights.
         Quadrature::QuadratureArray m_weights  [ 3 ];

         /// Indicator of whether iteration has completed.
         bool                        m_iterationComplete;

      };


      static Quadrature3D& getInstance ();

      ///  \brief Finalise the quadrature singlton.
      static void finalise ();

      /// \brief Get an iterator over the element volume for the specified degree.
      void get ( const int       degree,
                       Iterator& iter );

      /// \brief Get an iterator over the face of element for the specified degree.
      void get ( const int       degree,
                 const int       face,
                       Iterator& iter );

      /// \brief Get an iterator of the element volume with all three degrees specified.
      void get ( const int       xDegree,
                 const int       yDegree,
                 const int       zDegree,
                       Iterator& iter );

      // void get ( const int       xDegree,
      //            const int       yDegree,
      //            const int       zDegree,
      //            const int       face,
      //                  Iterator& iter );



   private :

      Quadrature3D () {}

      ~Quadrature3D () {}

      static Quadrature3D* s_quadratureInstance;


   };


}

//------------------------------------------------------------//
//          Inline functions                                  //
//------------------------------------------------------------//

inline NumericFunctions::Quadrature3D& NumericFunctions::Quadrature3D::getInstance () {

   if ( s_quadratureInstance == 0 ) {
      s_quadratureInstance = new Quadrature3D;
   }

   return *s_quadratureInstance;
}

inline double NumericFunctions::Quadrature3D::Iterator::getX () const {
   return m_points [ 0 ][ m_position [ 0 ]];
}

inline double NumericFunctions::Quadrature3D::Iterator::getY () const {
   return m_points [ 1 ][ m_position [ 1 ]];
}

inline double NumericFunctions::Quadrature3D::Iterator::getZ () const {
   return m_points [ 2 ][ m_position [ 2 ]];
}

inline double NumericFunctions::Quadrature3D::Iterator::getWeight () const {
   return m_weights [ 0 ][ m_position [ 0 ]] * m_weights [ 1 ][ m_position [ 1 ]] * m_weights [ 2 ][ m_position [ 2 ]];
}

inline bool NumericFunctions::Quadrature3D::Iterator::isDone () const {
   return m_iterationComplete;
}

#endif // _NUMERIC_FUNCTIONS__QUADRATURE_THREE_DIMENSION__H_
