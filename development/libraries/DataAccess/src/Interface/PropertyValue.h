#ifndef _INTERFACE_PROPERTYVALUE_H_
#define _INTERFACE_PROPERTYVALUE_H_

#include <sys/types.h>

#include <string>
using namespace std;

#include "Interface/DAObject.h"

#include "Interface/Interface.h"

namespace database
{
   class Database;
   class Table;
   class Record;
}

namespace DataAccess
{
   namespace Interface
   {
      /// A PropertyValue contains a map of values for a Property at a
      /// given Snapshot and Surface or Formation or Reservoir.
      /// If the Property of a PropertyValue is a RESERVOIRPROPERTY,
      /// the PropertyValue will refer to a Reservoir.
      /// If the Property of a PropertyValue is a FORMATIONPROPERTY and is discontinuous across formations,
      /// the PropertyValue will refer at least to a Formation and possibly to a Surface.
      /// If the Property of a PropertyValue is a FORMATIONPROPERTY and is continuous across formations,
      /// the PropertyValue will refer either to a Formation or to a Surface.
      class PropertyValue : public DAObject
      {
	 public:
	    PropertyValue (ProjectHandle * projectHandle, database::Record * record,
		  const string & name, const Property * property, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage);

	    virtual ~PropertyValue ();

            bool matchesConditions (int selectionFlags, const Property * property, const Snapshot * snapshot,
                  const Reservoir * reservoir, const Formation * formation,
                  const Surface * surface, int propertyType) const;

	    virtual GridMap * createGridMap (const Grid * grid, unsigned int depth = 0);

	    /// Return the name of this PropertyValue
	    virtual const string & getName (void) const;
	    /// Return the Snapshot for which this PropertyValue was computed
	    virtual const Snapshot * getSnapshot (void) const;
	    /// Return the Snapshot for which this PropertyValue was computed
	    virtual const Property * getProperty (void) const;
	    /// Return the Reservoir for which this PropertyValue was computed
	    /// if this PropertyValue applies to a Reservoir
	    virtual const Reservoir * getReservoir (void) const;
	    /// Return the Formation for which this PropertyValue was computed
	    /// if this PropertyValue applies to a Formation
	    virtual const Formation * getFormation (void) const;
	    /// Return the Surface for which this PropertyValue was computed
	    /// if this PropertyValue applies to a Surface
	    virtual const Surface * getSurface (void) const;
	    /// Return the GridMap that contains the values of this PropertyValue
	    virtual GridMap * getGridMap (void) const;
	    /// Return the GridMap that contains the values of this PropertyValue if already there
	    virtual GridMap * hasGridMap (void) const;

	    /// return the storage type
            inline virtual PropertyStorage getStorage (void) const;

            /// Indicate whether the property should be saved or not.
            ///
            /// Returns true, since in general all created property values are to be saved.
            virtual bool toBeSaved () const;

	    /// create a TimeIoRecord for this newly created and not yet saved PropertyValue
	    //bool createTimeIoRecord (database::Table * timeIoTbl);
            virtual database::Record* createTimeIoRecord (database::Table * timeIoTbl, ModellingMode theMode);

	    /// create a 1DTimeIoRecord for this newly created and not yet saved PropertyValue
            virtual database::Record* create1DTimeIoRecord (database::Table * timeIoTbl, ModellingMode theMode);

	    /// create a 3DTimeIoRecord for this newly created and not yet saved PropertyValue
            virtual database::Record* create3DTimeIoRecord (database::Table * timeIoTbl, ModellingMode theMode);

	    /// Link to a SnapshotIoRecord for this newly created and not yet saved PropertyValue
	    bool linkToSnapshotIoRecord (void);

	    /// save a 2D PropertyValue to file
	    virtual bool saveMapToFile (MapWriter & mapWriter);
	    /// save a 3D PropertyValue to file
	    virtual bool saveVolumeToFile (MapWriter & mapWriter);

	    /// save a primary 3D PropertyValue to file
	    virtual bool savePrimaryVolumeToFile (MapWriter & mapWriter);

	    /// Returns whether this PropertyValue has been uploaded onto a BPA server.
	    virtual bool isUploadedToBPA (void) const;
	    
	    string saveToDirectory (const string & directory);

	    /// First sorts on the age of the PropertyValue objects,
	    /// then on the deposition age of
	    /// the Surface objects that are, directly or indirectly,
	    /// associated with the PropertyValue objects.
	    static bool SortByAgeAndDepoAge (const PropertyValue * lhs, const PropertyValue * rhs);
	    /// Sorts on the deposition age of
	    /// the Surface objects that are, directly or indirectly,
	    /// associated with the PropertyValue objects.
	    static bool SortByDepoAge (const PropertyValue * lhs, const PropertyValue * rhs);

	    /// compare on the age of the PropertyValue objects
	    virtual int compareByAge (const PropertyValue * rhs) const;
	    /// First compare on the age of the PropertyValue objects,
	    /// then on the deposition age of
	    /// the Surface objects that are, directly or indirectly,
	    /// associated with the PropertyValue objects.
	    virtual int compareByAgeAndDepoAge (const PropertyValue * rhs) const;
	    /// Compares on the deposition age of
	    /// the Surface objects that are, directly or indirectly,
	    /// associated with the PropertyValue objects.
	    virtual int compareByDepoAge (const PropertyValue * rhs) const;

	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

	    /// Return true if the property has a record in a project file
	    bool hasRecord (void) const;
      protected:

            double getMode1DResult() const;

            static const unsigned int ValueMap = 0;

	    const string m_name;

	    const Property * m_property;
	    const Snapshot * m_snapshot;
	    const Reservoir * m_reservoir;
	    const Formation * m_formation;
	    const Surface * m_surface;

	    PropertyStorage m_storage;
      };

      PropertyStorage PropertyValue::getStorage (void) const
      {
	 return m_storage;
      }
   }
}


#endif // _INTERFACE_PROPERTYVALUE_H_
