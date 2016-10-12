#ifndef _INTERFACE_SERIALGRIDMAP_H_
#define _INTERFACE_SERIALGRIDMAP_H_

#include <iostream>
using namespace std;


#include <string>
using namespace std;

#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/Child.h"
namespace DataAccess
{
   namespace Interface
   {
 	
      /// A gridded map of values.
      /// Objects of this class contain the actual values of PropertyValue objects used or produced
      /// by a Cauldron run.
      /// A GridMap object is usually owned by another object, e.g. a PropertyValue or a Formation, which is responsible for
      /// its destruction
      class SerialGridMap: public GridMap
      {
	 public:
	    /// Create a GridMap from the given grid with the given value to be used for undefined values.
	    SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values);
        /// Create a constant GridMap from the given value
	    SerialGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth = 1);
     /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified operator function.
     SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator);
     /// Create a GridMap from the given GridMap after elementwise processing by the specified operator function.
     SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator);

  	   /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
	    SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor );
	    /// Create a GridMap from the two given GridMap objects after elementwise processing by the specified binary function object.
	    SerialGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, UnaryFunctor& unaryFunctor );

	   	/// retrieve data including ghost points
	    virtual bool retrieveGhostedData (void) const;
	    
   		/// retrieve data in-or excluding ghost points according to specification
	    virtual bool retrieveData (bool withGhosts = false) const;

	    /// restore data according to specifications
	    virtual bool restoreData (bool save = true, bool withGhosts = false) const;
	   
   		/// set the value for the given grid coordinates
	    virtual bool setValue (unsigned int i, unsigned int j, double value);

	    /// set the value for the given grid coordinates
	    virtual bool setValue (unsigned int i, unsigned int j, unsigned int k, double value);

   		/// get the value for the given index values
	    virtual double getValue (unsigned int i, unsigned int j) const;

   		/// get the value for the given index values
	    virtual double getValue (unsigned int i, unsigned int j, unsigned int k) const;

	    /// get the value for the given index values.
	    virtual double getValue (unsigned int i, unsigned int j, double k) const;

	    /// get the value for the given index values.
	    /// Interpolate if necessary.
	    virtual double getValue (double i, double j, double k) const;

	    /// return a fraction of the specified value
	    virtual double getFractionalValue (double fraction, unsigned int i, unsigned int j, unsigned int k = 0) const;

	    /// Return whether the value at the given index values is defined
	    virtual bool valueIsDefined (unsigned int i, unsigned int j, unsigned int k = 0) const;

	    /// return the average value calculated over all GridPoints with a defined value
	    virtual double getAverageValue (void) const;

	    /// return the minimum & maximum value calculated over all GridPoints with a defined value
	    virtual void getMinMaxValue (double & min, double & max) const;

	    /// return the sum of values
	    virtual double getSumOfValues (void) const;

	    /// return the sum of squared values
	    virtual double getSumOfSquaredValues (void) const;

	    /// return the number of defined values
	    virtual int getNumberOfDefinedValues (void) const;

	    /// return the value used for undefined values
	    virtual double getUndefinedValue (void) const;

     /// Check if GridMap is constant
     virtual bool isConstant (void) const;

     /// Return the constant value. returns the undefined value if not constant
	    virtual double getConstantValue (void) const;

	    /// Return the Grid on which this GridMap is based.
	    /// This Grid will either be the input grid, the low resolution output grid or
	    /// the high resolution output grid.
	    virtual const Grid * getGrid (void) const;

	    /// Return the vertical depth (i.e. value of the third dimension) of this GridMap.
	    /// This value equals 1 if the GridMap is two-dimensional and is larger than 1 if
	    /// the GridMap is three-dimensional
	    virtual unsigned int getDepth (void) const;

	    /// cease usage of this object
	    /// Informs the Parent object that the user application has stopped using this GridMap so that
	    /// it may be deleted.
	    virtual void release (void) const;

	    /// Return the array of values managed by this GridMap
	    virtual double const * const * const * getValues (void) const;

	    /// Save this gridmap in the HDF5 format
	    virtual bool saveHDF5 (const string & fileName) const;

	    /// Print the attributes and map values of this GridMap
            // May not work if user application is compiled under IRIX with CC -lang:std
	    virtual void printOn (ostream &) const;

	    /// map interpolation functionality
	    virtual bool convertToGridMap(GridMap *mapB) const; 
	    
	    /// Transforms a high res local grid map to a low res local grid map 
	    virtual bool transformHighRes2LowRes (GridMap *mapB) const;
	    /// Transforms a low res local grid map to a high res local grid map 
	    virtual bool transformLowRes2HighRes (GridMap *mapB) const;
		
		   virtual ~SerialGridMap (void);
	 
     /// return the local horizontal resolution
     unsigned int numI (void) const;
     /// return the local vertical resolution
     unsigned int numJ (void) const;

     /// return the leftmost local grid coordinate value
     double minI (void) const;
     /// return the bottommost local grid coordinate value
     double minJ (void) const;

     /// return the distance between two vertical gridlines
     double deltaI (void) const;
     /// return the distance between two horizontal gridlines
     double deltaJ (void) const;
     
     bool retrieved() const { return true;}
 
     /// return false 
     bool isAscendingOrder() const;
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
