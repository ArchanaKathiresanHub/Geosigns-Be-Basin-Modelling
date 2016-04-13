#ifndef _INTERFACE_PROPERTY_H_
#define _INTERFACE_PROPERTY_H_

#include <sys/types.h>

#include <string>
using namespace std;

#include "AbstractProperty.h"
#include "PropertyAttribute.h"

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   //using MAP;
   //using VOLUME;

   namespace Interface
   {
      /// A Property describes a property that can be found in the model.
      /// A Property either applies to Formation objects, in which case its type is FORMATIONPROPERTY
      /// or it applies to Reservoir objects, in which case its type is RESERVOIRPROPERTY.
      class Property : public DataModel::AbstractProperty, public DAObject
      {
	 public:
	    Property (ProjectHandle * projectHandle, database::Record * record,
		  const string & userName, const string & cauldronName,
                      const string & unit, PropertyType type,
                      const DataModel::PropertyAttribute attr );
	    virtual ~Property (void);

	    /// returns whether a Property has PropertyValues matching the conditions specified
	    /// if an argument equals 0, it is used as a wildcard
	    virtual bool hasPropertyValues (int selectionFlags,
		  const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation,
		  const Surface * surface, int propertyType = MAP | VOLUME) const;

	    /// Return the name of this Property
	    virtual const string & getName (void) const;
	    virtual const string & getUserName (void) const;
	    virtual const string & getCauldronName (void) const;
	    /// Return the unit of this Property
	    virtual const string & getUnit (void) const;
	    virtual PropertyType getType (void) const;

            /// \brief Get the PropertyAttribute of the property.
            virtual DataModel::PropertyAttribute getPropertyAttribute () const;

	    /// return a list of PropertyValues for this property based on the given arguments.
	    /// if an argument (not selectionFlags) equals 0, it is used as a wildcard
	    /// The arguments \b property and \b snapshot are used in the search for all kinds of propertyValues.
	    /// \arg \b selectionFlags \n
	    /// If (SelectionFlag & FORMATION) is true, look for Formation Properties.
	    /// The argument \b formation is used to find properties of this kind.\n
	    /// If (SelectionFlag & SURFACE) is true, look for Surface Properties that are
	    /// continuous across Surfaces.
	    /// The argument \b surface is used to find properties of this kind.\n
	    /// If (SelectionFlag & FORMATIONSURFACE) is true, look for Surface Properties that are
	    /// discontinuous across Surfaces.
	    /// The arguments \b formation and \b surface is used to find properties of this kind.\n
	    /// If (SelectionFlag & RESERVOIR) is true, look for Reservoir Properties.
	    /// The argument \b reservoir is used to find properties of this kind.\n
	    virtual PropertyValueList * getPropertyValues (int selectionFlags,
		  const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation,
		  const Surface * surface) const;

	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

         bool isPrimary() const;

	 private:
	    string m_userName;
	    string m_cauldronName;
	    string m_unit;
    
         bool m_isPrimaryProperty;

            PropertyType m_type;
         DataModel::PropertyAttribute m_propertyAttribute;
  
     };
   }
}


#endif // _INTERFACE_PROPERTY_H_
