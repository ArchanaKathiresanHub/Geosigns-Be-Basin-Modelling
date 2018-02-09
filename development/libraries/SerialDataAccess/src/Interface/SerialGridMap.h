// 
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_SERIALGRIDMAP_H_
#define _INTERFACE_SERIALGRIDMAP_H_

#include <string>

#include "Interface/Interface.h"
#include "Interface/GridMap.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A gridded map of values.
      /// Objects of this class contain the actual values of PropertyValue objects used or produced
      /// by a Cauldron run.
      /// A GridMap object is usually owned by another object, e.g. a PropertyValue or a Formation, which is responsible for
      /// its destruction
      class SerialGridMap : public GridMap
      {
      public:
         /// Create a GridMap from the given grid with the given value to be used for undefined values.
         SerialGridMap(const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values);
         /// Create a constant GridMap from the given value
         SerialGridMap(const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth = 1);
         /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified operator function.
         SerialGridMap(const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator);
         /// Create a GridMap from the given GridMap after elementwise processing by the specified operator function.
         SerialGridMap(const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator);

         /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
         SerialGridMap(const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor);
         /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
         SerialGridMap(const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryFunctor& unaryFunctor);

         ~SerialGridMap() final;

         /// retrieve data including ghost points
         bool retrieveGhostedData() const final;

         /// retrieve data in-or excluding ghost points according to specification
         bool retrieveData(bool withGhosts = false) const final;

         /// restore data according to specifications
         bool restoreData(bool save = true, bool withGhosts = false) const final;

         /// set the value for the given grid coordinates
         bool setValue(unsigned int i, unsigned int j, double value) final;

         /// set the value for the given grid coordinates
         bool setValue(unsigned int i, unsigned int j, unsigned int k, double value) final;

         /// set the constant value for the whole grid
         void setValues(const double value);

         /// get the value for the given index values
         double getValue(unsigned int i, unsigned int j) const final;

         /// get the value for the given index values
         double getValue(unsigned int i, unsigned int j, unsigned int k) const final;

         /// get the value for the given index values.
         double getValue(unsigned int i, unsigned int j, double k) const final;

         /// get the value for the given index values.
         /// Interpolate if necessary.
         double getValue(double i, double j, double k) const final;

         /// return a fraction of the specified value
         double getFractionalValue(double fraction, unsigned int i, unsigned int j, unsigned int k = 0) const final;

         /// Return whether the value at the given index values is defined
         bool valueIsDefined(unsigned int i, unsigned int j, unsigned int k = 0) const final;

         /// return the average value calculated over all GridPoints with a defined value
         double getAverageValue() const final;

         /// return the minimum & maximum value calculated over all GridPoints with a defined value
         void getMinMaxValue(double & min, double & max) const final;

         /// return the sum of values
         double getSumOfValues() const final;

         /// return the sum of squared values
         double getSumOfSquaredValues() const final;

         /// return the number of defined values
         int getNumberOfDefinedValues() const final;

         /// return the value used for undefined values
         double getUndefinedValue() const final;

         /// Check if GridMap is constant
         bool isConstant() const final;

         /// Return the constant value. returns the undefined value if not constant
         double getConstantValue() const final;

         /// Return the Grid on which this GridMap is based.
         /// This Grid will either be the input grid, the low resolution output grid or
         /// the high resolution output grid.
         const Grid * getGrid() const final;

         /// Return the vertical depth (i.e. value of the third dimension) of this GridMap.
         /// This value equals 1 if the GridMap is two-dimensional and is larger than 1 if
         /// the GridMap is three-dimensional
         unsigned int getDepth() const final;

         /// cease usage of this object
         /// Informs the Parent object that the user application has stopped using this GridMap so that
         /// it may be deleted.
         void release() const final;

         /// Return the array of values managed by this GridMap
         double const * const * const * getValues() const final;

         /// Save this gridmap in the HDF5 format
         bool saveHDF5(const string & fileName) const final;

         /// Print the attributes and map values of this GridMap
              // May not work if user application is compiled under IRIX with CC -lang:std
         void printOn(ostream &) const final;

         /// map interpolation functionality
         bool convertToGridMap(GridMap *mapB) const final;

         /// Transforms a high res local grid map to a low res local grid map 
         bool transformHighRes2LowRes(GridMap *mapB) const final;
         /// Transforms a low res local grid map to a high res local grid map 
         bool transformLowRes2HighRes(GridMap *mapB) const final;


         /// return the local horizontal resolution
         unsigned int numI() const final;
         /// return the local vertical resolution
         unsigned int numJ() const final;

         /// return the leftmost local grid coordinate value
         double minI() const final;
         /// return the bottommost local grid coordinate value
         double minJ() const final;

         /// return the distance between two vertical gridlines
         double deltaI() const final;
         /// return the distance between two horizontal gridlines
         double deltaJ() const final;

         bool retrieved() const final { return true; }

         /// return false 
         bool isAscendingOrder() const final;
      private:
         /// The (regular) Grid this GridMap is based on.
         const Grid * m_grid;

         /// In case the GridMap is constant
         double m_singleValue;

         /// three-dimensional array of values
         mutable double *** m_values;

         /// The value used in case the Map is undefined a some points
         const double m_undefinedValue;

         /// The eaverage Map value
         mutable double m_averageValue;

         /// The depth of the third dimension
         unsigned int m_depth;
      };
   }
}

#endif // _INTERFACE_GRIDMAP_H_
