//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_GRIDMAP_H
#define INTERFACE_GRIDMAP_H

#include <iostream>
using namespace std;

#include <string>
using namespace std;

#include "Interface.h"
#include "Child.h"
namespace DataAccess
{
   namespace Interface
   {
      /// A gridded map of values.
      /// Objects of this class contain the actual values of PropertyValue objects used or produced
      /// by a Cauldron run.
      class GridMap: public Child
      {
      public:
         /// Create a GridMap from the given grid with the given value to be used for undefined values.
         GridMap (const Parent * owner, unsigned int childIndex): Child (owner, childIndex) { };
         
         /// retrieve data including ghost points
         virtual bool retrieveGhostedData (void) const = 0;
         
         /// retrieve data in-or excluding ghost points according to specification
         virtual bool retrieveData (bool withGhosts = false) const = 0;
         
         /// restore data according to specifications
         virtual bool restoreData (bool save = true, bool withGhosts = false) const = 0;
         
         /// set the value for the given grid coordinates
         virtual bool setValue (unsigned int i, unsigned int j, double value) = 0;
         
         /// set the value for the given grid coordinates
         virtual bool setValue (unsigned int i, unsigned int j, unsigned int k, double value) = 0;
         
         /// get the value for the given index values
         virtual double getValue (unsigned int i, unsigned int j) const = 0;
         
         /// get the value for the given index values
         virtual double getValue (unsigned int i, unsigned int j, unsigned int k) const = 0;
         
         /// get the value for the given index values.
         virtual double getValue (unsigned int i, unsigned int j, double k) const = 0;
         
         /// get the value for the given index values.
         /// Interpolate if necessary.
         virtual double getValue (double i, double j, double k = 0) const = 0;
         
         /// return a fraction of the specified value
         virtual double getFractionalValue (double fraction, unsigned int i, unsigned int j, unsigned int k = 0) const = 0;
         
         /// Return whether the value at the given index values is defined
         virtual bool valueIsDefined (unsigned int i, unsigned int j, unsigned int k = 0) const = 0;
         
         /// return the average value calculated over all GridPoints with a defined value
         virtual double getAverageValue (void) const = 0;
         
         /// return the minimum & maximum value calculated over all GridPoints with a defined value
         virtual void getMinMaxValue (double & min, double & max) const = 0;
         
         /// return the sum of values
         virtual double getSumOfValues (void) const = 0;
         
         /// return the sum of squared values
         virtual double getSumOfSquaredValues (void) const = 0;
         
         /// return the number of defined values
         virtual int getNumberOfDefinedValues (void) const = 0;
         
         /// return the value used for undefined values
         virtual double getUndefinedValue (void) const = 0;
         
         /// Check if GridMap is constant
         virtual bool isConstant (void) const = 0;
         
         /// Return the constant value. returns the undefined value if not constant
         virtual double getConstantValue (void) const = 0;
         
         /// Return the Grid on which this GridMap is based.
         /// This Grid will either be the input grid, the low resolution output grid or
         /// the high resolution output grid.
         virtual const Grid * getGrid (void) const = 0;
         
         /// Return the vertical depth (i.e. value of the third dimension) of this GridMap.
         /// This value equals 1 if the GridMap is two-dimensional and is larger than 1 if
         /// the GridMap is three-dimensional
         virtual unsigned int getDepth (void) const = 0;
         
         /// cease usage of this object
         /// Informs the Parent object that the user application has stopped using this GridMap so that
         /// it may be deleted.
         virtual void release (void) const = 0;
         
         /// Return the array of values managed by this GridMap
         virtual double const * const * const * getValues (void) const = 0;
         
         /// Save this gridmap in the HDF5 format
         virtual bool saveHDF5 (const string & fileName) const = 0;
         
         /// Print the attributes and map values of this GridMap
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (ostream &) const = 0;
         
         /// map interpolation functionality
         virtual bool convertToGridMap(GridMap *mapB) const = 0; 
         
         /// Transforms a high res local grid map to a low res local grid map 
         virtual bool transformHighRes2LowRes (GridMap *mapB) const = 0;
         /// Transforms a low res local grid map to a high res local grid map 
         virtual bool transformLowRes2HighRes (GridMap *mapB) const = 0;
         
         /// destructor
         virtual ~GridMap (void) {};

         /// return the local horizontal resolution
         virtual unsigned int numI (void) const = 0;
         /// return the local vertical resolution
         virtual unsigned int numJ (void) const = 0;

         /// return the first local horizontal index
         unsigned int firstI (void) const;
         /// return the first local vertical index
         unsigned int firstJ (void) const;
         /// return the first local thickness index
         unsigned int firstK (void) const;

         /// return the first local horizontal index
         unsigned int firstI ( bool withGhosts ) const;
         /// return the first local vertical index
         unsigned int firstJ ( bool withGhosts ) const;

         /// return the last local horizontal index
         unsigned int lastI (void) const;
         /// return the last local vertical index
         unsigned int lastJ (void) const;
         /// return the last local thickness index
         unsigned int lastK (void) const;

         /// return the last local horizontal index
         unsigned int lastI( bool withGhosts ) const;
         /// return the last local vertical index
         unsigned int lastJ( bool withGhosts ) const;

         /// return the leftmost local grid coordinate value
         virtual double minI (void) const = 0;
         /// return the bottommost local grid coordinate value
         virtual double minJ (void) const = 0;

         /// return the distance between two vertical gridlines
         virtual double deltaI (void) const = 0;
         /// return the distance between two horizontal gridlines
         virtual double deltaJ (void) const = 0;

         virtual bool retrieved() const = 0;
          
          /// return true if the data are stored in ascending order
         virtual bool isAscendingOrder() const = 0;
      };
   }
}

#endif // INTERFACE_GRIDMAP_H
