//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _PARALLELDATAACCESS_LOCALGRIDMAP_H_
#define _PARALLELDATAACCESS_LOCALGRIDMAP_H_

#include <iostream>
#include <string>

#include "petscvec.h"
#include "petscdmda.h"

#include "Interface/Interface.h"
#include "Interface/GridMap.h"


namespace DataAccess
{
   namespace Interface
   {
      // Forward declaration
      class DistributedGrid;

      /// A gridded map of values.
      /// Objects of this class contain the actual values of PropertyValue objects used or produced
      /// by a Cauldron run.
      /// A GridMap object is usually owned by another object, e.g. a PropertyValue or a Formation, which is responsible for
      /// its destruction
      class DistributedGridMap : public GridMap
      {
      public:

      using GridMap::firstI;
      using GridMap::firstJ;
      using GridMap::lastI;
      using GridMap::lastJ;
      using GridMap::minI;
      using GridMap::minJ;

      /// Create GridMap according to specified grid,
      /// with the given depth and undefined value.
      DistributedGridMap( const Grid * grid,
                          const unsigned int depth,
                          const double & undefinedValue );
      /// Create a constant GridMap from the given value
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const Grid * grid,
                          const double & value,
                          const unsigned int depth = 1 );
      /// Create a GridMap from the given grid with the given array of values
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const Grid * grid,
                          const double & undefinedValue,
                          const unsigned int depth,
                          float *** const values );
      /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified operator function.
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const GridMap * operand1,
                          const GridMap * operand2,
                          BinaryOperator binaryOperator );
      /// Create a GridMap from the given GridMap after elementwise processing by the specified operator function.
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const GridMap * operand,
                          UnaryOperator unaryOperator );
      /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const GridMap * operand1,
                          const GridMap * operand2,
                          BinaryFunctor & binaryFunctor );
      /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
      DistributedGridMap( const Parent * owner,
                          const unsigned int childIndex,
                          const GridMap * operand1,
                          UnaryFunctor & unaryFunctor );

      /// destructor
      ~DistributedGridMap (void);

      /// retrieve data including ghost points
      bool retrieveGhostedData (void) const;
      /// retrieve data in-or excluding ghost points according to specification
      bool retrieveData (bool withGhosts = false) const;

      /// restore data according to specifications
      bool restoreData (bool save = true, bool withGhosts = false) const;
         
      /// return whether the map is in ascending order or not
      bool isAscendingOrder() const;

      /// Return the Petsc Vec
      Vec & getVec (void);
      /// Return the Petsc DA
      DM & getDA (void) const;

      /// return the leftmost local grid coordinate value
      double minI (void) const;
      /// return the bottommost local grid coordinate value
      double minJ (void) const;

      /// return the rightmost local grid coordinate value
      double maxI (void) const;
      /// return the topmost local grid coordinate value
      double maxJ (void) const;

      /// return the local horizontal resolution
      unsigned int numI (void) const;
      /// return the local vertical resolution
      unsigned int numJ (void) const;

      /// return the leftmost local grid coordinate value
      double minI (bool withGhosts) const;
      /// return the bottommost local grid coordinate value
      double minJ (bool withGhosts) const;

      /// return the rightmost local grid coordinate value
      double maxI (bool withGhosts) const;
      /// return the topmost local grid coordinate value
      double maxJ (bool withGhosts) const;

      /// return the local horizontal resolution
      unsigned int numI (bool withGhosts) const;
      /// return the local vertical resolution
      unsigned int numJ (bool withGhosts) const;

      /// return the first local horizontal index
      unsigned int firstI (bool withGhosts) const;
      /// return the first local vertical index
      unsigned int firstJ (bool withGhosts) const;

      /// return the last local horizontal index
      unsigned int lastI (bool withGhosts) const;
      /// return the last local vertical index
      unsigned int lastJ (bool withGhosts) const;


      /// return the distance between two vertical gridlines
      double deltaI (void) const;
      /// return the distance between two horizontal gridlines
      double deltaJ (void) const;

      /// return whether this point is a valid local grid point
      bool isGridPoint (unsigned int i, unsigned int j, unsigned int k) const;

      /// set the value for the given grid coordinates
      bool setValue (unsigned int i, unsigned int j, double value);

      /// set the value for the given grid coordinates
      bool setValue (unsigned int i, unsigned int j, unsigned int k, double value);

      /// get the value for the given grid coordinates
      virtual double getValue (unsigned int i, unsigned int j) const;

      /// get the value for the given grid coordinates
      virtual double getValue (unsigned int i, unsigned int j, unsigned int k) const;

      /// get the value for the given grid coordinates
      virtual double getValue (unsigned int i, unsigned int j, double k) const;

      /// get the value for the given index values.
      /// Interpolate if necessary.
      virtual double getValue (double i, double j, double k = 0) const;

      /// return a fraction of the specified value
      double getFractionalValue (double fraction, unsigned int i, unsigned int j, unsigned int k = 0) const;

      /// get the average value for all points, collective function
      virtual double getAverageValue () const;

      /// return the minimum & maximum value calculated over all GridPoints with a defined value
      virtual void getMinMaxValue (double & min, double & max) const;

      /// return the sum of values
      virtual double getSumOfValues (void) const;

      /// return the sum of squared values
      virtual double getSumOfSquaredValues (void) const;

      /// return the number of defined values
      virtual int getNumberOfDefinedValues (void) const;

      /// Return whether the value at the given index values is defined
      virtual bool valueIsDefined (unsigned int i, unsigned int j, unsigned int k = 0) const;

      /// return the value used for undefined values
      virtual double getUndefinedValue (void) const;

      /// Check if GridMap is constant
      virtual bool isConstant (void) const;

      /// Return the constant value. returns the undefined value if not constant
      virtual double getConstantValue (void) const;

      /// return the underlying grid
      virtual const Grid *getGrid (void) const;

      /// return the depth (size) of the third dimension of the GridMap
      virtual unsigned int getDepth (void) const;

      /// cease usage of this object
      /// Informs the Parent object that the user application has stopped using this GridMap so that
      /// it may be deleted.
      virtual void release (void) const;

      /// Return the array of values managed by this GridMap
      virtual double const * const * const * getValues (void) const;

      virtual bool saveHDF5 (const std::string & fileName) const;
      virtual void printOn (std::ostream &) const;
      virtual void printOn (MPI_Comm comm) const;
    
      ///map interpolation functionality
      bool convertToGridMap(GridMap *mapB) const;

      bool retrieved() const { return m_retrieved; }

      private:
      
      // Prevent copy constructor and assignement
      DistributedGridMap & operator=(const DistributedGridMap & other);
      DistributedGridMap (const DistributedGridMap & other);

      /// initialization
      void initialize (void);

      /// The (regular) Grid this GridMap is based on.
      const Grid *m_grid;

      /// three-dimensional array of values
      mutable double ***m_values;

      /// The value used in case the Map is undefined a some points
      const double m_undefinedValue;

      /// The average Map value
      mutable double m_averageValue;

      /// The depth of the third dimension
      unsigned int m_depth;

      mutable DMDALocalInfo m_localInfo;

      /// The PETSC Global VEC
      Vec m_vecGlobal;

      /// The PETSC Local VEC
      mutable Vec m_vecLocal;

      /// whether the data was retrieved with or without ghosts.
      mutable bool m_withGhosts;

      /// whether data was retrieved from the global gridmap.
      mutable bool m_retrieved;

      /// whether data was changed after retrieval
      mutable bool m_modified;
    
      ///Transforms a high res local grid map to a low res local grid map 
      bool transformHighRes2LowRes (GridMap *mapB) const;
      ///Transforms a low res local grid map to a high res local grid map 
      bool transformLowRes2HighRes (GridMap *mapB) const;
      bool findLowResElementCoordinates( const unsigned int HighResI,
                                         const unsigned int HighResJ,
                                         const unsigned int depth,
                                         const DistributedGridMap * lowResMap,
                                         const DistributedGrid * lowResGrid,
                                         const DistributedGrid * HighResGrid,
                                         unsigned int lowResElementCoordinatesInHighRes[],
                                         double nodalValuesInLowResElement[],
                                         bool useGhostNodesInLowRes );

      //utility for findLowResElementCoordinates
      bool isHighResNodeInLowResElement( const unsigned int & HighResI,
                                         const unsigned int & HighResJ,
                                         unsigned int lowResElementCoordinatesInHighRes[] );

      bool isHighResNodeInLowResGrid( const unsigned int & HighResI,
                                      const unsigned int & HighResJ,
                                      const GridMap * lowResMap,
                                      const Grid * lowResGrid,
                                      const Grid * highResGrid );
      };

      typedef GridMap LocalGridMap;
   }
}

#endif   // _PARALLELDATAACCESS_LOCALGRIDMAP_H_
