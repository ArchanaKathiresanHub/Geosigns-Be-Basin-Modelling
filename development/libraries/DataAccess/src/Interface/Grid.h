#ifndef _INTERFACE_GRID_H_
#define _INTERFACE_GRID_H_

#include <iostream>
#include <string>

#include "AbstractGrid.h"

namespace DataAccess
{
   namespace Interface
   {
      /// Class describing regular grids.
      /// Objects of this class describe the input and output grids of a Cauldron project.
      class Grid : public DataModel::AbstractGrid
      {
      public:
         /// create a new grid
         //Grid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ);
         //Grid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ);
         virtual ~Grid (void) {};

         /// Checks whether the given point is a grid point
         virtual bool isGridPoint (unsigned int i, unsigned int j) const = 0;

         /// return the leftmost grid coordinate value
         virtual double minI (void) const = 0;
         /// return the bottommost grid coordinate value
         virtual double minJ (void) const = 0;
         /// return the rightmost grid coordinate value
         virtual double maxI (void) const = 0;
         /// return the topmost grid coordinate value
         virtual double maxJ (void) const = 0;
         /// return the horizontal resolution
         virtual int numI (void) const = 0;
         /// return the vertical resolution
         virtual int numJ (void) const = 0;
         /// return the first local horizontal index
         virtual int firstI (void) const = 0;
         virtual int firstI (bool withGhosts) const = 0;
         /// return the first local vertical index
         virtual int firstJ (void) const = 0;
         virtual int firstJ (bool withGhosts) const = 0;
         /// return the last local horizontal index
         virtual int lastI (void) const = 0;
         virtual int lastI (bool withGhosts) const = 0;
         /// return the last local vertical index
         virtual int lastJ (void) const = 0;
         virtual int lastJ (bool withGhosts) const = 0;


         /// return the distance between two vertical gridlines
         virtual double deltaI (void) const = 0;
         /// return the distance between two horizontal gridlines
         virtual double deltaJ (void) const = 0;

         /// return the leftmost grid coordinate value
         virtual double minIGlobal (void) const = 0;
         /// return the bottommost grid coordinate value
         virtual double minJGlobal (void) const = 0;
         /// return the rightmost grid coordinate value
         virtual double maxIGlobal (void) const = 0;
         /// return the topmost grid coordinate value
         virtual double maxJGlobal (void) const = 0;
         /// return the horizontal resolution
         virtual int numIGlobal (void) const = 0;
         /// return the vertical resolution
         virtual int numJGlobal (void) const = 0;

         /// return the distance between two vertical gridlines
         virtual double deltaIGlobal (void) const = 0;
         /// return the distance between two horizontal gridlines
         virtual double deltaJGlobal (void) const = 0;


         /// return the number of processors in the I direction
         virtual int numProcsI (void) const = 0;
         /// return the number of processors in the J direction
         virtual int numProcsJ (void) const = 0;

         /// return the number of grid points per processor in the I direction
         virtual int * numsI (void) const = 0;
         /// return the number of grid points per processor in the J direction
         virtual int * numsJ (void) const = 0;

         /// returns true if local grid is situated left of global grid
         virtual bool onLowISide (void) const = 0;
         /// returns true if local grid is situated right of global grid
         virtual bool onHighISide (void) const = 0;

         /// returns true if local grid is situated at bottom of global grid
         virtual bool onLowJSide (void) const = 0;
         /// returns true if local grid is situated at top of global grid
         virtual bool onHighJSide (void) const = 0;

         /// return the gridpoint with the given real-world coordinates
         virtual bool getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) const = 0;

         /// return the gridpoint with the given real-world coordinates
         virtual bool getGridPoint (double posI, double posJ, double & i, double & j) const = 0;

         /// get the real-world position of the given gridpoint
         virtual bool getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const = 0;

         /// get the real world position of the given point in the grid
         virtual bool getPosition (double i, double j, double &posI, double &posJ) const = 0;

         /// convert grid point to grid point of specified grid
         virtual bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
                                     unsigned int &toI, unsigned int &toJ, bool useCaching = true) const = 0;

         virtual bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
                                     double &toI, double &toJ, bool useCaching = true) const = 0;

         /// get the realworld surface at the given grid coordinates
         virtual double getSurface (unsigned int i, unsigned int j) const = 0;
         /// get the real-world distance between the two given gridpoints
         virtual double getDistance (unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const = 0;

         virtual void printOn (std::ostream &) const = 0;
         virtual void asString (std::string&) const = 0;

         virtual bool isEqual( const Grid& grid ) const = 0;

      };

      inline bool operator==(const Grid & Grid1, const Grid & Grid2) { return Grid1.isEqual( Grid2 ); }
   }
}

#endif // _INTERFACE_GRID_H_
