// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_SERIALGRID_H_
#define _INTERFACE_SERIALGRID_H_

#include <iostream>
using namespace std;

#include "Interface/Interface.h"
#include "Interface/Grid.h"

namespace DataAccess
{
   namespace Interface
   {
      /// Class describing regular grids.
      /// Objects of this class describe the input and output grids of a Cauldron project.
      class SerialGrid: public Grid  
      {
      public:
         SerialGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ);
         SerialGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ);
         ~SerialGrid (void);
         /// Checks whether the given point is a grid point
         virtual bool isGridPoint (unsigned int i, unsigned int j) const;
         
         /// return the leftmost grid coordinate value
         virtual double minI (void) const;
         /// return the bottommost grid coordinate value
         virtual double minJ (void) const;
         /// return the rightmost grid coordinate value
         virtual double maxI (void) const;
         /// return the topmost grid coordinate value
         virtual double maxJ (void) const;
         /// return the horizontal resolution
         virtual int numI (void) const;
         /// return the vertical resolution
         virtual int numJ (void) const;
         /// return the first local horizontal index
         virtual int firstI (void) const;
         virtual int firstI (bool withGhosts) const;
         /// return the first local vertical index
         virtual int firstJ (void) const;
         virtual int firstJ (bool withGhosts) const;
         /// return the last local horizontal index
         virtual int lastI (void) const;
         virtual int lastI (bool withGhosts) const;
         /// return the last local vertical index
         virtual int lastJ (void) const;
         virtual int lastJ (bool withGhosts) const;
         
         /// return the distance between two vertical gridlines
         virtual double deltaI (void) const;
         /// return the distance between two horizontal gridlines
         virtual double deltaJ (void) const;
         
         /// return the leftmost grid coordinate value
         virtual double minIGlobal (void) const;
         /// return the bottommost grid coordinate value
         virtual double minJGlobal (void) const;
         /// return the rightmost grid coordinate value
         virtual double maxIGlobal (void) const;
         /// return the topmost grid coordinate value
         virtual double maxJGlobal (void) const;
         /// return the horizontal resolution
         virtual int numIGlobal (void) const;
         /// return the vertical resolution
         virtual int numJGlobal (void) const;
         
         /// return the distance between two vertical gridlines
         virtual double deltaIGlobal (void) const;
         /// return the distance between two horizontal gridlines
         virtual double deltaJGlobal (void) const;
         
         /// return the number of processors in the I direction
         virtual int numProcsI (void) const;
         /// return the number of processors in the J direction
         virtual int numProcsJ (void) const;
         
         /// return the number of grid points per processor in the I direction
         int * numsI (void) const;
         /// return the number of grid points per processor in the J direction
         int * numsJ (void) const;
         
         /// returns true if local grid is situated left of global grid
         bool onLowISide (void) const;
         /// returns true if local grid is situated right of global grid
         bool onHighISide (void) const;
         
         /// returns true if local grid is situated at bottom of global grid
         bool onLowJSide (void) const;
         /// returns true if local grid is situated at top of global grid
         bool onHighJSide (void) const;
         
         /// return the gridpoint with the given real world coordinates
         virtual bool getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) const;
         
         /// return the gridpoint with the given real-world coordinates
         virtual bool getGridPoint (double posI, double posJ, double & i, double & j) const;
         
         /// get the real world position of the given gridpoint
         virtual bool getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const;
         
         /// get the real world position of the given point in the grid, interpolate if necessary
         virtual bool getPosition (double i, double j, double &posI, double &posJ) const;
         
         /// Get the real-world surface at the given grid point.
         /// The surface covered at the side points of the grid will be
         /// half the surface covered by the internal points of the grid and
         /// the surface covered at the corner points of the grid will be
         /// a quarter of the surface covered by the internal points of the grid.
         virtual double getSurface (unsigned int i, unsigned int j) const;
         /// get the real world distance between the two given gridpoints
         virtual double getDistance (unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const;
         
         /// convert grid point to grid point of specified grid
         virtual bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
         unsigned int &toI, unsigned int &toJ,  bool useCaching = true) const;
         
         /// convert grid point to grid point of specified grid
         virtual bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
         double &toI, double &toJ,  bool useCaching = true) const;
         
         /// Print the attributes of this Grid
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (ostream &) const;
         virtual void asString (std::string&) const;
         
         bool hasSameGridding( const Grid& grid ) const;

      protected:
         //virtual ~SerialGrid (void);
      private:
         const double m_minI;
         const double m_minJ;
         const double m_maxI;
         const double m_maxJ;
         const unsigned int m_numI;
         const unsigned int m_numJ;
         int * m_numsI;
         int * m_numsJ;
         const double m_deltaI;
         const double m_deltaJ;
         const double m_surface;
      };
   }
}

#endif // _INTERFACE_SERIALGRID_H_
