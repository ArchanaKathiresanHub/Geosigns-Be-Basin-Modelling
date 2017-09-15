#include <math.h>
#include <assert.h>

#include "Interface/FaultElementCalculator.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/FaultCollection.h"
#include "Interface/Faulting.h"

#include <algorithm>

#include "Interface/auxiliaryfaulttypes.h"

using namespace DataAccess;
using namespace Interface;


//------------------------------------------------------------//

FaultElementCalculator::FaultElementCalculator (void) : m_grid (nullptr), m_snapshot (nullptr)
{
}

//------------------------------------------------------------//

void FaultElementCalculator::setGrid ( const Grid * grid )
{
  m_grid = grid;
}

void FaultElementCalculator::setSnapshot ( const Snapshot * snapshot )
{
  m_snapshot = snapshot;
}

bool FaultElementCalculator::computeFaultGridMap (GridMap * faultMap, FaultCollectionList * fcList, const Snapshot * snapshot)
{
   setGrid ((Grid *) faultMap->getGrid ());
   setSnapshot (snapshot);

   FaultCollectionList::iterator fcIter;

   for (fcIter = fcList->begin (); fcIter != fcList->end (); ++fcIter)
   {
      FaultCollection * fc = (FaultCollection *) * fcIter;
      computeFaultGridMap (faultMap, fc);
   }
   
   return true;
}

bool FaultElementCalculator::computeFaultGridMap (GridMap * faultMap, FaultCollection * fc)
{
   FaultList * faults = ((FaultCollection *) fc)->getFaults ();

   FaultList::iterator faultIter;
   for (faultIter = faults->begin (); faultIter != faults->end (); ++faultIter)
   {
      Fault * fault = (Fault *) * faultIter;

      ElementSet faultElements;

      computeFaultElements (fault->getFaultLine (), faultElements);

      FaultStatus faultStatus = fault->getStatus (m_snapshot);

      ElementSet::iterator elementIter;

      for (elementIter = faultElements.begin (); elementIter != faultElements.end (); ++elementIter)
      {
         faultMap->setValue ((*elementIter) (X_COORD), (*elementIter) (Y_COORD), double (faultStatus));
      }
   }

   delete faults;

   return true;
}


//------------------------------------------------------------//

void LinearPolynomial::setPoints ( const double P1,
                                                           const double P2,
                                                           const double Distance ) {

  assert ( Distance > 0.0 );

  M = ( P2 - P1 ) / Distance;
  C = P1;

}

//------------------------------------------------------------//

double LinearPolynomial::computeP ( const double S ) const {
  return M * S + C;
}

//------------------------------------------------------------//

double LinearPolynomial::inverseP ( const double P ) const {
  return ( P - C ) / M;
}

//------------------------------------------------------------//

double LinearPolynomial::slope () const {
  return M;
}

//------------------------------------------------------------//

double LinearPolynomial::constant () const {
  return C;
}

//------------------------------------------------------------//

void LineSegment::setPoints ( const Point& P1, 
                                                      const Point& P2 ) {

  Start = P1;
  End   = P2;

  double R = separationDistance ( P1, P2 );
  xPoly.setPoints ( P1 ( X_COORD ), P2 ( X_COORD ), R );
  yPoly.setPoints ( P1 ( Y_COORD ), P2 ( Y_COORD ), R );
  
}

//------------------------------------------------------------//

double LineSegment::computeY ( const double X ) const {

  double S;

  S = xPoly.inverseP ( X );

  return yPoly.computeP ( S );
}

//------------------------------------------------------------//

double LineSegment::computeX ( const double Y ) const {

  double S;

  S = yPoly.inverseP ( Y );

  return xPoly.computeP ( S );
}

//------------------------------------------------------------//

const Point LineSegment::startPoint () const {
  return Start;
}

//------------------------------------------------------------//

const Point LineSegment::endPoint () const {
  return End;
}

//------------------------------------------------------------//

bool LineSegment::allignedWithXGridLine () const {
  return yPoly.slope () == 0.0;
}

//------------------------------------------------------------//

bool LineSegment::allignedWithYGridLine () const {
  return xPoly.slope () == 0.0;
}

//------------------------------------------------------------//

bool LineSegment::allignedWithGridLines () const {
  return allignedWithXGridLine () || allignedWithYGridLine ();
}

//------------------------------------------------------------//

void LineSegment::print () const {
//    std::cout << " Y = " << M << " X + " << C << endl;
//    std::cout << " X = " << " ( Y - " << C << " ) / " << M << endl;
}

//------------------------------------------------------------//

void PointLessThan::setPoint ( const Point& P ) {
  startPoint ( X_COORD ) = P ( X_COORD );
  startPoint ( Y_COORD ) = P ( Y_COORD );
}

//------------------------------------------------------------//

bool PointLessThan::operator() ( const Point& P1,
                                 const Point& P2 ) const {

  return distanceFromStart ( P1 ) < distanceFromStart ( P2 );
}

//------------------------------------------------------------//

double PointLessThan::distanceFromStart ( const Point& P ) const {
  return separationDistance ( startPoint, P );
}

//------------------------------------------------------------//

void FaultElementCalculator::computeFaultElements ( const PointSequence& faultLine,
                                                          ElementSet&    faultElements ) const
{
   int I;

   faultElements.clear ();

   for (I = 1; I < int (faultLine.size ()); I++)
   {
      ///
      /// Compute the elements for every sub-segment in the fault-line.
      ///
      getElementsTraversed (faultLine[I - 1], faultLine[I], faultElements);
   }

}

//------------------------------------------------------------//

void FaultElementCalculator::computeGridIntersectionPoints ( const LineSegment&   LS,
                                                                   PointSequence& gridIntersectionPoints,
                                                                   bool&          translatedNeeded ) const
{
   int I;
   size_t oldSize;
   size_t newSize;

   Point P;
   PointLessThan PLT;
   Element startElement;
   Element endElement;

   ///
   /// Set the line segment, this computes the polynomial describing the line
   ///
   PLT.setPoint (LS.startPoint ());

   ///
   /// Which elements do the end points lie in?
   ///
   startElement = whichElement (*m_grid, LS.startPoint ());
   endElement = whichElement (*m_grid, LS.endPoint ());

#if 0
   cerr << "Start Point = (" << LS.startPoint () (X_COORD) << ", " << LS.startPoint () (Y_COORD) << ")" << endl;
   cerr << "End Point = (" << LS.endPoint () (X_COORD) << ", " << LS.endPoint () (Y_COORD) << ")" << endl;
   cerr << "Start Element = (" << startElement (X_COORD) << ", " << startElement (Y_COORD) << ")" << endl;
   cerr << "End Element = (" << endElement (X_COORD) << ", " << endElement (Y_COORD) << ")" << endl;
#endif

   ///
   /// Find x start. To simplify things when finding the y-grid-line-intersection 
   /// points, order the startElement and endElement's x-coordinate index.
   ///
   if (startElement (X_COORD) > endElement (X_COORD))
   {
      std::swap (startElement (X_COORD), endElement (X_COORD));
   }

   ///
   /// Find y start. To simplify things when finding the x-grid-line-intersection 
   /// points, order the startElement and endElement's x-coordinate index.
   ///
   if (startElement (Y_COORD) > endElement (Y_COORD))
   {
      std::swap (startElement (Y_COORD), endElement (Y_COORD));
   }

   startElement (X_COORD) = startElement (X_COORD) + 1;
   startElement (Y_COORD) = startElement (Y_COORD) + 1;

   ///
   /// Add the line segment end points to the sequence of points ...
   ///
   gridIntersectionPoints.push_back (LS.startPoint ());
   gridIntersectionPoints.push_back (LS.endPoint ());

   ///
   /// Compute the positions at which the line-segment intersects the y-grid lines.
   ///
   ///
   ///  +-------+-------+-------+-------+-------+
   ///  |       |       |       |       | O     |
   ///  |   O   |       |       |       X       |
   ///  |   |   |       |       |     / |       |
   ///  +---|---+-------+-------+---/---+-------+
   ///  |   |   |       |       | /     |       |
   ///  |   |   |       |       X       |       |
   ///  |   |   |       |     / |       |       |
   ///  +---|---+-------+---/---+-------+-------+
   ///  |   |   |       | /     |       |       |
   ///  |   |   |       X       |       |       |
   ///  |   |   |     / |       |       |       |
   ///  +---|---+---/---+-------+-------+-------+
   ///  |   |   | O     |       |       |       |
   ///  |   O   |  O----X-------X-------X---O   |
   ///  |       |       |       |       |       |
   ///  +-------+-------+-------+-------+-------+
   ///
   ///
   for (I = startElement (X_COORD); I <= endElement (X_COORD); I++)
   {
      P (X_COORD) = m_grid->minIGlobal () + double (I) * m_grid->deltaI ();

      P (Y_COORD) = LS.computeY (P (X_COORD));
      gridIntersectionPoints.push_back (P);
   }

   ///
   /// Compute the positions at which the line-segment intersects the x-grid lines.
   ///
   ///  +-------+-------+-------+-------+-------+
   ///  |       |       |       |       | O     |
   ///  |   O   |       |       |       /       |
   ///  |   |   |       |       |     / |       |
   ///  +---X---+-------+-------+---X---+-------+
   ///  |   |   |       |       | /     |       |
   ///  |   |   |       |       /       |       |
   ///  |   |   |       |     / |       |       |
   ///  +---X---+-------+---X---+-------+-------+
   ///  |   |   |       | /     |       |       |
   ///  |   |   |       /       |       |       |
   ///  |   |   |     / |       |       |       |
   ///  +---X---+---X---+-------+-------+-------+
   ///  |   |   | O     |       |       |       |
   ///  |   O   |  O----+-------+-------+---O   |
   ///  |       |       |       |       |       |
   ///  +-------+-------+-------+-------+-------+
   ///
   ///
   for (I = startElement (Y_COORD); I <= endElement (Y_COORD); I++)
   {
      P (Y_COORD) = m_grid->minJGlobal () + double (I) * m_grid->deltaJ ();

      P (X_COORD) = LS.computeX (P (Y_COORD));
      gridIntersectionPoints.push_back (P);
   }

   ///
   /// Sort the points in distance from the start of the line segment
   ///
   std::sort (gridIntersectionPoints.begin (), gridIntersectionPoints.end (), PLT);
   oldSize = gridIntersectionPoints.size ();

   ///
   /// Now remove duplicate points
   ///
   gridIntersectionPoints.erase (std::unique (gridIntersectionPoints.begin (), gridIntersectionPoints.end ()),
                                 gridIntersectionPoints.end ());
   newSize = gridIntersectionPoints.size ();

   ///
   /// Only need to compute the translated line IFF the line-segment is not alligned with 
   /// the grid lines and some points that lie along the line-segment are not unique.
   /// (The last clause can be expressed because the x-axis and y-axis intersection points
   ///  are computed separately, and so may appear on the set more than once)
   ///
   translatedNeeded = (!LS.allignedWithGridLines () && oldSize != newSize);

}

//------------------------------------------------------------//

void FaultElementCalculator::getElementsTraversed ( const LineSegment& LS,
                                                          ElementSet&  ES,
                                                          bool&        translatedNeeded ) const
{
   int I;

   Element E;
   Point P;
   PointSequence gridLineIntersectionPoints;

   computeGridIntersectionPoints (LS, gridLineIntersectionPoints, translatedNeeded);

   ///
   /// Compute the mid-points of the line sub-segments, then find which element this point lies in.
   ///
   for (I = 1; I < int (gridLineIntersectionPoints.size ()); I++)
   {
      P = midPoint (gridLineIntersectionPoints[I - 1], gridLineIntersectionPoints[I]);
      E = whichElement (*m_grid, P);
      ES.insert (E);
   }

}

//------------------------------------------------------------//

void FaultElementCalculator::getElementsTraversed ( const Point&      P1,
                                                    const Point&      P2,
                                                          ElementSet& ES ) const
{
   if (P1 == P2)
   {
      ES.insert (whichElement (*m_grid, P1));
   }
   else
   {
      const double EpsilonX = 1.0e-4 * m_grid->deltaI ();
      const double EpsilonY = 1.0e-4 * m_grid->deltaJ ();

      bool translatedNeeded;
      double Length;
      LineSegment LS;

      Point segmentDirection;
      Point segmentNormal;
      Point segmentDisplacement;

      Point P1Tilde;
      Point P2Tilde;

      ///
      /// Add the end points of the fault line-segment
      ///
      ES.insert (whichElement (*m_grid, P1));
      ES.insert (whichElement (*m_grid, P2));

      ///
      /// Compute the element set between the points p1 and p2
      ///
      LS.setPoints (P1, P2);
      getElementsTraversed (LS, ES, translatedNeeded);

      ///
      /// If needed then the line must be translated slightly from its original 
      /// position and the elements that the line intersects re-evaluated.
      ///
      if (translatedNeeded)
      {

         ///
         /// Vector describing direction of the line between points p1 and p2.
         ///
         segmentDirection = P1 - P2;

         ///
         /// The normal to the line between p1 and p2.
         ///
         segmentNormal (X_COORD) = segmentDirection (Y_COORD);
         segmentNormal (Y_COORD) = -segmentDirection (X_COORD);

         Length = sqrt (segmentNormal (X_COORD) * segmentNormal (X_COORD) +
                        segmentNormal (Y_COORD) * segmentNormal (Y_COORD));

         ///
         /// Dividing by length to make unit normal.
         ///
         segmentNormal = segmentNormal / Length;

         ///
         /// How far from the original line should the new line be displaced?
         ///
         segmentDisplacement (X_COORD) = EpsilonX * segmentNormal (X_COORD);
         segmentDisplacement (Y_COORD) = EpsilonY * segmentNormal (Y_COORD);

         ///
         /// Compute the end points of the slightly displaced line
         ///
         P1Tilde = P1 + segmentDisplacement;
         P2Tilde = P2 + segmentDisplacement;

         ///
         /// Compute the element set of the slightly displaced line (p1~ and p2~)
         ///
         LS.setPoints (P1Tilde, P2Tilde);
         getElementsTraversed (LS, ES, translatedNeeded);
      }
   }

}

//------------------------------------------------------------//
