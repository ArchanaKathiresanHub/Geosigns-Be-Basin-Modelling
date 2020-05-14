//
// Copyright (C) 2015-2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_AUXILIARY_FAULT_TYPES_H
#define _INTERFACE_AUXILIARY_FAULT_TYPES_H

#include <iostream>
#include <string>
#include <vector>
#include <set>


#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      class Grid;

      enum DistanceUnit { METRE, FOOT };


      void convertDistanceUnits ( const DistanceUnit From,
            const DistanceUnit To,
            double&      Value );


      ///
      /// A point in R^3
      ///
      class Point
      {
         private :
            double m_values [ 3 ];

         public :
            explicit Point () :
              m_values{0.0, 0.0, 0.0}
            {
            }

            ~Point (void)
            {
            }

            Point (const Point & point)
            {
               m_values[X_COORD] = point.m_values[X_COORD];
               m_values[Y_COORD] = point.m_values[Y_COORD];
               m_values[Z_COORD] = point.m_values[Z_COORD];
            }

            Point(double x, double y, double z)
            {
              m_values[X_COORD] = x;
              m_values[Y_COORD] = y;
              m_values[Z_COORD] = z;
            }

            void operator = ( const Point& point )
            {
              m_values[X_COORD] = point.m_values[X_COORD];
              m_values[Y_COORD] = point.m_values[Y_COORD];
              m_values[Z_COORD] = point.m_values[Z_COORD];
            }

            double  operator()( const CoordinateAxis axis ) const
            {
               return m_values [ axis ];
            }

            double& operator[]( const CoordinateAxis axis )  {
               return m_values [ axis ];
            }
      };

      ///
      /// Some operations on a Point.
      ///
      Point midPoint ( const Point& P1, const Point& P2 );

      ostream& operator<< ( ostream& o, const Point& P );

      bool operator == ( const Point& P1, const Point& P2 );

      bool operator != ( const Point& P1, const Point& P2 );

      Point operator - ( const Point& P1, const Point& P2 );

      Point operator + ( const Point& P1, const Point& P2 );

      Point operator * ( const double X, const Point& P1 );

      Point operator / ( const Point& P1, const double X );

      double separationDistance ( const Point& P1, const Point& P2 );

      typedef std::vector<Point> PointSequence;

      typedef const PointSequence& PointSequenceConstRef;

      ///
      /// The indices of an element.
      ///
      class Element {

         public :

            int  operator()( const CoordinateAxis axis ) const {
               return m_values [ axis ];
            }

            int& operator()( const CoordinateAxis axis )  {
               return m_values [ axis ];
            }


            friend ostream& operator<< ( ostream& o, const Element& E );


         private :

            int m_values [ 2 ];

      };

      Element whichElement ( const Grid & grid,
                             const Point& P );

      ///
      /// Order the elements, first by the X_Coordinate, then the Y_Coordinate.
      ///
      class ElementLess {

         public :

            bool operator()( const Element& E1, const Element& E2 ) const;

      };


      typedef std::set<Element, ElementLess > ElementSet;

      bool operator== ( const Element& E1, const Element& E2 );
      bool operator!= ( const Element& E1, const Element& E2 );

      ostream& operator << ( ostream& o, const ElementSet& ES );

      ostream& operator << ( ostream& o, const PointSequence& PS );


      ///
      /// Transmissibility of the fault with respect to water.
      ///
      enum PressureFaultStatus { PASS_WATER, SEAL_WATER };

      std::string pressurefaultStatusImage ( const PressureFaultStatus faultStatus );
   }
}


#endif // _INTERFACE_AUXILIARY_FAULT_TYPES_H

