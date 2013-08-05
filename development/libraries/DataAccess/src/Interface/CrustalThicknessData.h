#ifndef _INTERFACE_CRUSTALTHICKNESS_DATA_H_
#define _INTERFACE_CRUSTALTHICKNESS_DATA_H_

using namespace std;

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {

      class CrustalThicknessData : public DAObject
      {
	 public:
	    CrustalThicknessData (ProjectHandle * projectHandle, database::Record * record);
	    virtual ~CrustalThicknessData (void);

            /// Return the T0 
            virtual const double & getT0Ini(void) const;
            /// Return the TR
            virtual const double & getTRIni(void) const;
            /// Return the Initial Crustal Thickness
            virtual const double & getHCuIni(void) const;
            /// Return the Initial mantle thickness
            virtual const double & getHLMuIni(void) const;
            /// Return the Maximum Basaltic thickness
            virtual const double & getHBu(void) const;
            /// Return the Sealevel adjustment
            virtual const double & getDeltaSL(void) const;
            /// Return the Filter width (half)
            virtual const int & getFilterHalfWidth(void) const;
            /// Return the name of a base of syn-rift 
            virtual const string & getSurfaceName(void) const;

	    /// Return the (GridMap) value of one of this Crustal Thickness Calculator's attributes
	    virtual const Interface::GridMap * getMap (Interface::CTCMapAttributeId attributeId) const;

	    /// load a map
	    GridMap * loadMap (Interface::CTCMapAttributeId attributeId) const;

	    /// Print the parameters of this Crustal Thickness Calculator
	    virtual void printOn (ostream &) const;

	    /// Print the parameters of this Crustal Thickness Calculator
	    virtual void asString (string &) const;

	 private:
	    static const string s_MapAttributeNames[];
      };
   }
}


#endif // _INTERFACE_CRUSTALTHICKNESS_DATA_H_
