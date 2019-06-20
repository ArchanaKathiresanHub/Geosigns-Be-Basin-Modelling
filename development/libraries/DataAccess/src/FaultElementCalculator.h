#ifndef _FAULT_ELEMENT_CALCULATOR_H
#define _FAULT_ELEMENT_CALCULATOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "auxiliaryfaulttypes.h"
#include "Interface.h"

//------------------------------------------------------------//

namespace DataAccess
{
   namespace Interface
   {
      class Grid;
      class GridMap;

      class LinearPolynomial {

         public :

            LinearPolynomial () {}

            void setPoints ( const double P1, const double P2, const double Distance );

            double computeP ( const double S ) const;

            double inverseP ( const double P ) const;

            double slope () const;

            double constant () const;

         private :

            ///
            /// P(s) = M * s + C
            ///
            double M;
            double C;

      };

      class LineSegment {

         public :

            void setPoints ( const Point& P1, 
                  const Point& P2 );

            double computeY ( const double X ) const;

            double computeX ( const double Y ) const;


            void print () const;


            const Point startPoint () const;

            const Point endPoint   () const;


            bool allignedWithXGridLine () const;

            bool allignedWithYGridLine () const;

            ///
            /// aligned with either the x-axis or the y-axis.
            ///
            bool allignedWithGridLines () const;


         private :

            Point  Start;
            Point  End;

            ///
            /// The line segment is parameterised by the distance from the start point.
            ///
            LinearPolynomial xPoly;
            LinearPolynomial yPoly;

      };


      ///
      /// Functor class to allow sorting of points in order of distance from 'start point'
      /// Acts like a 'less than', returning || P1 - SP || < || P2 - SP ||
      ///
      class PointLessThan {

         public :

            void setPoint ( const Point& P );

            bool operator() ( const Point& P1, const Point& P2 ) const;

         private :

            double distanceFromStart ( const Point& P ) const;

            Point startPoint;

      };


      class FaultElementCalculator {

         public :

            FaultElementCalculator ();

            void setGrid (const Grid * grid);
            void setSnapshot (const Snapshot * snapshot);

            bool computeFaultGridMap (GridMap * faultMap, FaultCollectionList * fcList, const Snapshot * snapshot);
            bool computeFaultGridMap (GridMap * faultMap, FaultCollection * fc);

            ///
            /// Returns the set of elements that the fault line intersects.
            ///
            void computeFaultElements ( const PointSequence& faultLine,
                  ElementSet&    faultElements ) const;


         private :

            void computeGridIntersectionPoints ( const LineSegment&   LS,
                  PointSequence& Points,
                  bool&          translatedNeeded ) const;

            void getElementsTraversed ( const LineSegment& LS,
                                        ElementSet&  ES,
                                        bool&        translatedNeeded ) const;

            ///
            /// Compute the elements that the line, connecting points P1 and P2, intersects.
            ///
            void getElementsTraversed ( const Point&      P1,
                  const Point&      P2,
                  ElementSet& ES ) const;

            const Grid * m_grid;
            const Snapshot * m_snapshot;
      };
   }
}

#endif // _FAULT_ELEMENT_CALCULATOR_H

