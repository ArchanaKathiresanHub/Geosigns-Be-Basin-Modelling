#ifndef _INTERFACE_OUTPUTPROPERTY_H_
#define _INTERFACE_OUTPUTPROPERTY_H_

#include <iostream>
using namespace std;

#include <sys/types.h>
#include <string>
using namespace std;

#include "Property.h"
#include "DAObject.h"
#include "Interface.h"


namespace DataAccess
{
   namespace Interface
   {

      /// An OutputProperty object contains the name of the property, the modelling mode
      /// for the property, and indicates the required level of output.
      class OutputProperty : public DAObject
      {
	 public:

	    OutputProperty (ProjectHandle * projectHandle, database::Record * record);

	    OutputProperty (ProjectHandle * projectHandle, const ModellingMode mode, const PropertyOutputOption option, const std::string& name );

	    virtual ~OutputProperty (void);

	    /// Return the name of this OutputProperty.
	    virtual const string & getName (void) const;

	    /// Return the modelling-mode of this OutputProperty.
            virtual ModellingMode getMode (void) const;

	    /// Return the output option of this OutputProperty.
	    virtual PropertyOutputOption getOption (void) const;

	    /// Set a new output option of this OutputProperty.
	    virtual void setOption ( const PropertyOutputOption newOption );

	    /// Print the attributes of this Property
            // May not work if user application is compiled under IRIX with CC -lang:std
	    virtual void printOn (ostream &) const;

            virtual void asString (string & str) const;

	 protected:

            const std::string m_name;
            ModellingMode        m_mode;
            PropertyOutputOption m_option;

      };
   }
}


#endif // _INTERFACE_OUTPUTPROPERTY_H_
