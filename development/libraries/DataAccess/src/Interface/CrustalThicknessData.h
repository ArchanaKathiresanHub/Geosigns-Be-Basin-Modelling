//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _INTERFACE_CRUSTALTHICKNESS_DATA_H_
#define _INTERFACE_CRUSTALTHICKNESS_DATA_H_

using namespace std;

// DataAccess library
#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {

      /// @class CrustalThicknessData The CTC interface
      class CrustalThicknessData : public DAObject
      {
	      public:
	         CrustalThicknessData (ProjectHandle * projectHandle, database::Record * record);
	         virtual ~CrustalThicknessData (void);

            /// @brief Return the rifting event starting time (T0)
            virtual const double & getT0Ini(void) const;
            /// @brief Return the rifitng event end time (TR)
            virtual const double & getTRIni(void) const;
            /// @brief Return the last age at which the CTC will create outputs
            /// @details This represents the end of rifting and the beginning of the flexural basin
            virtual const double & getLastComputationAge( void ) const;
            /// @brief Return the Initial Crustal Thickness
            virtual const double & getHCuIni(void) const;
            /// @brief Return the Initial mantle thickness
            virtual const double & getHLMuIni(void) const;
            /// @brief Return the Maximum Basaltic thickness
            virtual const double & getHBu(void) const;
            /// @brief Return the Sealevel adjustment
            virtual const double & getDeltaSL(void) const;
            /// @brief Return the Filter width (half), this is the smoothing radius
            virtual const int & getFilterHalfWidth(void) const;
            /// @brief Return the name of a base of syn-rift 
            virtual const string & getSurfaceName(void) const;

	    /// @brief Return the (GridMap) value of one of this Crustal Thickness Calculator's attributes
       /// @param attributeId The map ID
	    virtual const Interface::GridMap * getMap (Interface::CTCMapAttributeId attributeId) const;

	    /// @brief Load a CTC map
       /// @param attributeId The map ID
	    GridMap * loadMap (Interface::CTCMapAttributeId attributeId) const;

	    /// @brief Print the parameters of this Crustal Thickness Calculator
	    virtual void printOn (ostream &) const;

	    /// @brief Print the parameters of this Crustal Thickness Calculator
	    virtual void asString (string &) const;

	 private:
	    static const char* s_MapAttributeNames[];   ///< The names of the CTC maps
      };
   }
}


#endif // _INTERFACE_CRUSTALTHICKNESS_DATA_H_
