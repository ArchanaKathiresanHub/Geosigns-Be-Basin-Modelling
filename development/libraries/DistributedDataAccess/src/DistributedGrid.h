// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _PARALLELDATAACCESS_LOCALGRID_H_
#define _PARALLELDATAACCESS_LOCALGRID_H_

#include "petscsys.h"
#include "petscvec.h"
#include "petscdmda.h"

#include <string>

#include "Interface.h"
#include "GlobalGrid.h"
#include "Grid.h"

namespace DataAccess
{
   namespace Interface
   {
      typedef int PetscMPIInt;
 
      /// Class describing processor-local regular grids.
      class DistributedGrid: public Grid
      {
      public:        
         /// create a new grid
        DistributedGrid (double minI, double minJ, double maxI, double maxJ, int numI, int numJ, int lowResNumI, int lowResNumJ,
                         const std::vector<std::vector<int> >& domainShape = {});
         ~DistributedGrid (void);
         
         /// Create a new grid such that each grid point and its corresponding grid point in the reference grid
         /// are assigned to the same processor
         /// The new grid is a downscaled version of the old grid
         DistributedGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, int numI, int numJ);
         
         /// Checks whether the given point is a grid point
         virtual bool isGridPoint (unsigned int i, unsigned int j) const;
         
         /// return the leftmost local grid coordinate value
         virtual double minI (void) const;
         double minI (bool withGhosts) const;
         /// return the bottommost local grid coordinate value
         virtual double minJ (void) const;
         double minJ (bool withGhosts) const;
         /// return the rightmost local grid coordinate value
         virtual double maxI (void) const;
         double maxI (bool withGhosts) const;
         /// return the topmost local grid coordinate value
         virtual double maxJ (void) const;
         double maxJ (bool withGhosts) const;
         /// return the local horizontal resolution
         int numI (void) const;
         int numI (bool withGhosts) const;
         /// return the local vertical resolution
         int numJ (void) const;
         int numJ (bool withGhosts) const;
         /// return the first local horizontal index
         int firstI (void) const;
         int firstI (bool withGhosts) const;
         /// return the first local vertical index
         int firstJ (void) const;
         int firstJ (bool withGhosts) const;
         /// return the last local horizontal index
         int lastI (void) const;
         int lastI (bool withGhosts) const;
         /// return the last local vertical index
         int lastJ (void) const;
         int lastJ (bool withGhosts) const;
         
         /// return the leftmost grid coordinate value
         double minIGlobal (void) const;
         /// return the bottommost grid coordinate value
         double minJGlobal (void) const;
         /// return the rightmost grid coordinate value
         double maxIGlobal (void) const;
         /// return the topmost grid coordinate value
         double maxJGlobal (void) const;
         /// return the horizontal resolution
         int numIGlobal (void) const;
         /// return the vertical resolution
         int numJGlobal (void) const;
         /// return the horizontal pitch
         double deltaIGlobal (void) const;
         /// return the vertical pitch
         double deltaJGlobal (void) const;
         
         
         /// return the number of processors in the I direction
         int numProcsI (void) const;
         /// return the number of processors in the J direction
         int numProcsJ (void) const;
         
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
         
         /// return the distance between two vertical gridlines
         double deltaI (void) const;
         /// return the distance between two horizontal gridlines
         double deltaJ (void) const;
         
         /// return the gridpoint with the given real-world coordinates
         bool getGridPoint (double posI, double posJ, unsigned int &i, unsigned int &j) const;
         
         /// return the gridpoint with the given real-world coordinates
         bool getGridPoint (double posI, double posJ, double & i, double & j) const;
         
         /// get the real-world position of the given gridpoint
         bool getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const;
         
         /// get the real world position of the given point in the grid
         bool getPosition (double i, double j, double &posI, double &posJ) const;
         
         /// convert grid point to grid point of specified grid
         bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
         unsigned int &toI, unsigned int &toJ) const;
         
         /// convert grid point to grid point of specified grid
         bool convertToGrid (const Grid & toGrid, unsigned int fromI, unsigned int fromJ,
          double &toI, double &toJ) const;
         
         /// get the realworld surface at the given grid coordinates
         double getSurface (unsigned int i, unsigned int j) const;
         
         /// return the rank of the processor that handles this grid point
         unsigned int getRank (unsigned int i, unsigned int j) const;
         
         /// get the real-world distance between the two given gridpoints
         double getDistance (unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const;
         
         /// return DALocalInfo;
         const DMDALocalInfo & getLocalInfo (void) const;
         
         /// Return the Petsc Vec
         const Vec & getVec (void) const;
         
         /// return DALocalInfo;
         const GlobalGrid & getGlobalGrid (void) const;
         
         virtual void asString (std::string &) const;
         virtual void printOn (std::ostream &) const;
         
         void printDistributionOn (MPI_Comm com) const;
         
         static bool checkForValidPartitioning (int M, int N);

         bool hasSameGridding( const Grid& grid ) const;

      private:
         GlobalGrid m_globalGrid;
         DMDALocalInfo m_localInfo;
         
         Vec m_vecGlobal;
         
         mutable int m_numProcsI;
         mutable int m_numProcsJ;
         
         mutable int *m_numsI;
         mutable int *m_numsJ;
         
         mutable bool * m_returnsI;
         mutable bool * m_returnsJ;
         
         void calculateNums (const Grid * referenceGrid);

         PetscErrorCode createPETSCDynamicDecomposition(int& numICores, int& numJCores, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);
         PetscErrorCode createPETSCStaticDecomposition(int& numICores, int& numJCores);
      };
   }
}

#endif // _PARALLELDATAACCESS_LOCALGRID_H_
