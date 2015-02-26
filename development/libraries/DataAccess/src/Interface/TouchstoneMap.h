#ifndef _INTERFACE_TOUCHSTONEMAP_H_
#define _INTERFACE_TOUCHSTONEMAP_H_

using namespace std;

#include "Interface/Interface.h"
#include "Interface/DAObject.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A TouchstoneMap object contains information on a specific source rock
      class TouchstoneMap : public DAObject
      {
	 public:
	    TouchstoneMap (ProjectHandle * projectHandle, database::Record * record);
	    virtual ~TouchstoneMap (void);

	    /// Return the Formation of this Map
	    virtual const Formation * getFormation (void) const;
	    /// Return the Surface of this Map
	    virtual const Surface * getSurface (void) const;
	    /// Return the name of the TCF
	    virtual const string & getTCFName (void) const;
	    /// Return the result category
	    virtual const string & getCategory (void) const;
	    /// Return the stochasitic property
	    virtual const string & getFormat (void) const;
	    /// Return the percentage in case Format is "Percentile"
	    virtual double getPercentage (void) const;
	    
	    /// Return the Facies Number
	    virtual int getFaciesNumber (void) const;
	    /// Return the name of the facies map
	    virtual const string & getFaciesNameMap (void) const;
		 /// Return the gridMap object
	    virtual const GridMap * getFaciesGridMap (void) const;

	    /// Find the PropertyValue that was produced for this TouchstoneMap
	    virtual const PropertyValue * findPropertyValue (void) const;

	    /// Print the attributes of this SourceRock
            // May not work if user application is compiled under IRIX with CC -lang:std
	    virtual void printOn (ostream &) const;

	    virtual void asString (string &) const;

	 private:
	    static const string s_MapAttributeNames[];
      };
   }
}


#endif // _INTERFACE_TOUCHSTONEMAP_H_
