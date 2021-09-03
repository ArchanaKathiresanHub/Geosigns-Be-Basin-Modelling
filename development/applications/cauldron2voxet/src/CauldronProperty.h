#ifndef _VOXET_CAULDRON_PROPERTY_H_
#define _VOXET_CAULDRON_PROPERTY_H_

#include <string>
#include <memory>

// Data-access
namespace DataAccess { namespace Interface {
   class ProjectHandle;
   class Property;
} }

// Table-io
namespace database {
   class Record;
}

/// A property that is computed and output by Cauldron.
class CauldronProperty {

public :

   CauldronProperty ( const DataAccess::Interface::ProjectHandle& cauldronProjectHandle,
                      database::Record*              record );

   /// The name of the property.
   const std::string& getCauldronName () const;

   /// The name of the property.
   const std::string& getVoxetName () const;

   /// The DAL cauldron property object.
   const DataAccess::Interface::Property* getProperty () const;

   /// Should this property be output in the voxet file.
   bool getVoxetOutput () const;

   /// What is the conversion factor for this property.
   double getConversionFactor () const;

   /// What are the units of this property.
   const std::string& getUnits () const;

private :

   const DataAccess::Interface::ProjectHandle& m_cauldronProjectHandle;
   database::Record*              m_record;


};

#endif // _VOXET_CAULDRON_PROPERTY_H_
